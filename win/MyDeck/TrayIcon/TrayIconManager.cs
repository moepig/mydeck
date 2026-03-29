namespace MyDeck.TrayIcon;

public sealed class TrayIconManager : IDisposable
{
    private readonly NotifyIcon _notify;

    public TrayIconManager(Action onSettings, Action onExit)
    {
        var iconPath = Path.Combine(AppContext.BaseDirectory, "assets", "tray.ico");
        var icon     = File.Exists(iconPath) ? new Icon(iconPath) : SystemIcons.Application;

        var menu = new ContextMenuStrip();
        menu.Items.Add("設定", null, (_, _) => onSettings());
        menu.Items.Add(new ToolStripSeparator());
        menu.Items.Add("終了", null, (_, _) => onExit());

        _notify = new NotifyIcon
        {
            Icon             = icon,
            Text             = "MyDeck - 切断中",
            ContextMenuStrip = menu,
            Visible          = true,
        };
    }

    public void SetConnected(bool connected)
    {
        _notify.Text = connected ? "MyDeck - 接続中" : "MyDeck - 切断中";
    }

    public void ShowBalloon(string title, string message)
    {
        _notify.ShowBalloonTip(2000, title, message, ToolTipIcon.Info);
    }

    public void Dispose()
    {
        _notify.Visible = false;
        _notify.Dispose();
    }
}
