using MyDeck.Command;
using MyDeck.Logging;
using NUnit.Framework;

namespace MyDeck.Tests.Command;

[TestFixture]
public class CommandExecutorTests
{
    private EventLog        _log      = null!;
    private CommandExecutor _executor = null!;

    [SetUp]
    public void SetUp()
    {
        _log      = new EventLog();
        _executor = new CommandExecutor(_log);
    }

    [Test]
    public void Execute_ValidProcess_AddsRunEntry()
    {
        _executor.Execute("cmd.exe", "/c exit 0");

        var entries = _log.Snapshot();
        Assert.That(entries, Has.Count.GreaterThanOrEqualTo(1));
        Assert.That(entries[0].Kind,   Is.EqualTo("実行"));
        Assert.That(entries[0].Detail, Does.Contain("cmd.exe"));
    }

    [Test]
    public async Task Execute_ValidProcess_AddsCompleteEntryWithExitCode()
    {
        var completed = new TaskCompletionSource();
        _log.Changed += () =>
        {
            if (_log.Snapshot().Any(e => e.Kind == "完了"))
                completed.TrySetResult();
        };

        _executor.Execute("cmd.exe", "/c exit 42");

        await completed.Task.WaitAsync(TimeSpan.FromSeconds(10));

        var entry = _log.Snapshot().First(e => e.Kind == "完了");
        Assert.That(entry.Detail, Does.Contain("42"));
    }

    [Test]
    public void Execute_InvalidExecutable_AddsErrorEntry()
    {
        _executor.Execute("nonexistent_program_xyz_abc.exe", "");

        var entries = _log.Snapshot();
        Assert.That(entries, Has.Count.EqualTo(1));
        Assert.That(entries[0].Kind, Is.EqualTo("エラー"));
    }

    [Test]
    public void Execute_InvalidExecutable_ErrorDetailContainsExecutableName()
    {
        _executor.Execute("nonexistent_program_xyz_abc.exe", "");

        Assert.That(_log.Snapshot()[0].Detail, Does.Contain("nonexistent_program_xyz_abc.exe"));
    }
}
