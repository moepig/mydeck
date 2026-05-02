using System.Diagnostics;

namespace MyDeck.Command;

public sealed class CommandExecutor : IMyDeckCommandExecutor
{
    public void Execute(string executable, string arguments)
    {
        var info = new ProcessStartInfo
        {
            FileName        = executable,
            Arguments       = arguments,
            UseShellExecute = true,
        };
        Process.Start(info);
    }
}
