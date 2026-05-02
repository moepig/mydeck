using System.Diagnostics;

namespace MyDeck.Command;

public sealed class CommandExecutor : IMyDeckCommandExecutor
{
    private readonly Logging.EventLog _log;

    public CommandExecutor(Logging.EventLog log)
    {
        _log = log;
    }

    public void Execute(string executable, string arguments)
    {
        var label = string.IsNullOrWhiteSpace(arguments) ? executable : $"{executable} {arguments}";
        try
        {
            var process = Process.Start(new ProcessStartInfo
            {
                FileName        = executable,
                Arguments       = arguments,
                UseShellExecute = true,
            });

            _log.Add(new Logging.EventLog.Entry(DateTime.Now, "実行", label));

            if (process is null) return;

            process.EnableRaisingEvents = true;
            process.Exited += (_, _) =>
            {
                try
                {
                    int code = process.ExitCode;
                    _log.Add(new Logging.EventLog.Entry(DateTime.Now, "完了", $"{label} → 終了コード {code}"));
                }
                catch { }
                finally { process.Dispose(); }
            };
        }
        catch (Exception ex)
        {
            _log.Add(new Logging.EventLog.Entry(DateTime.Now, "エラー", $"{label}: {ex.Message}"));
        }
    }
}
