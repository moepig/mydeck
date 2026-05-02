using MyDeck.Command;
using MyDeck.Config;
using MyDeck.Hid;
using MyDeck.Logging;
using MyDeck.Settings;
using MyDeck.TrayIcon;

namespace MyDeck;

public sealed class App : IDisposable
{
    private readonly string                   _configPath;
    private readonly IHidDevice               _hid;
    private readonly IMyDeckCommandExecutor   _executor;
    private readonly TrayIconManager          _tray;
    private readonly System.Threading.Timer   _reconnectTimer;
    private readonly byte[]                   _readBuf = new byte[8];
    private readonly SynchronizationContext   _uiContext;
    private readonly EventWaitHandle          _openSettingsEvent;
    private readonly EventLog                 _eventLog;
    private volatile CommandDispatcher        _dispatcher;
    private AppConfig                         _config;
    private CancellationTokenSource           _cts = new();
    private SettingsForm?                     _settingsForm;
    private EventLogForm?                     _eventLogForm;
    private bool _disposed;

    public App(AppConfig config, IHidDevice hid, IMyDeckCommandExecutor executor, EventLog eventLog, string configPath, string eventName)
    {
        _config            = config;
        _configPath        = configPath;
        _hid               = hid;
        _executor          = executor;
        _eventLog          = eventLog;
        _dispatcher        = new CommandDispatcher(executor, eventLog, config.Buttons);
        _tray              = new TrayIconManager(OpenSettings, OpenEventLog, Application.Exit);
        _uiContext         = SynchronizationContext.Current ?? new SynchronizationContext();
        _openSettingsEvent = new EventWaitHandle(false, EventResetMode.AutoReset, eventName);

        _reconnectTimer = new System.Threading.Timer(Reconnect, null, Timeout.Infinite, Timeout.Infinite);

        Task.Factory.StartNew(WatchOpenSettingsEvent, CancellationToken.None,
                              TaskCreationOptions.LongRunning, TaskScheduler.Default);
        TryConnect();
    }

    // ---- 接続管理 ----------------------------------------------

    private void TryConnect()
    {
        var dev = _config.Device;
        if (_hid.TryOpen(dev.VendorId, dev.ProductId))
        {
            _tray.SetConnected(true);
            _tray.ShowBalloon("MyDeck", "デバイスに接続しました。");
            _reconnectTimer.Change(Timeout.Infinite, Timeout.Infinite);
            StartReadLoop();
        }
        else
        {
            _tray.SetConnected(false);
            _reconnectTimer.Change(_config.Device.ReconnectIntervalMs,
                                   _config.Device.ReconnectIntervalMs);
        }
    }

    private void StartReadLoop()
    {
        _cts = new CancellationTokenSource();
        Task.Factory.StartNew(ReadLoop, _cts.Token,
                              TaskCreationOptions.LongRunning,
                              TaskScheduler.Default);
    }

    private void ReadLoop()
    {
        while (!_cts.IsCancellationRequested)
        {
            int n = _hid.Read(_readBuf);
            if (n < 0)
            {
                _tray.SetConnected(false);
                _reconnectTimer.Change(_config.Device.ReconnectIntervalMs,
                                       _config.Device.ReconnectIntervalMs);
                return;
            }
            if (n == 0) continue;

            var report = InputReport.TryParse(_readBuf);
            if (report is not null)
                _dispatcher.Dispatch(report);
        }
    }

    private void Reconnect(object? _)
    {
        if (!_hid.IsConnected)
            TryConnect();
    }

    // ---- 設定画面 ----------------------------------------------

    private void WatchOpenSettingsEvent()
    {
        while (!_disposed)
        {
            if (_openSettingsEvent.WaitOne(millisecondsTimeout: 1000) && !_disposed)
                _uiContext.Post(_ => OpenSettings(), null);
        }
    }

    public void OpenSettings()
    {
        if (_settingsForm is { IsDisposed: false })
        {
            _settingsForm.BringToFront();
            return;
        }
        _settingsForm = new SettingsForm(_config, _configPath, _eventLog);
        _settingsForm.ConfigSaved += newConfig =>
        {
            _config     = newConfig;
            _dispatcher = new CommandDispatcher(_executor, _eventLog, newConfig.Buttons);
        };
        _settingsForm.Show();
    }

    // ---- イベントログ画面 ---------------------------------------

    public void OpenEventLog()
    {
        if (_eventLogForm is { IsDisposed: false })
        {
            _eventLogForm.BringToFront();
            return;
        }
        _eventLogForm = new EventLogForm(_eventLog);
        _eventLogForm.Show();
    }

    // ---- 終了処理 ----------------------------------------------

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        _openSettingsEvent.Set();
        _cts.Cancel();
        _reconnectTimer.Dispose();
        _hid.Write(OutputReport.SetLed(false));
        _hid.Dispose();
        _tray.Dispose();
        _settingsForm?.Close();
        _eventLogForm?.Close();
        _openSettingsEvent.Dispose();
    }
}
