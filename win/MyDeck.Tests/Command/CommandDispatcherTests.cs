using MyDeck.Command;
using MyDeck.Config;
using MyDeck.Hid;
using MyDeck.Logging;
using NSubstitute;
using NUnit.Framework;

namespace MyDeck.Tests.Command;

[TestFixture]
public class CommandDispatcherTests
{
    private IMyDeckCommandExecutor _executor = null!;
    private EventLog               _log      = null!;
    private List<ButtonConfig>     _buttons  = null!;

    [SetUp]
    public void SetUp()
    {
        _executor = Substitute.For<IMyDeckCommandExecutor>();
        _log      = new EventLog();
        _buttons  =
        [
            new ButtonConfig
            {
                ButtonId = 1,
                Label    = "Test",
                Actions  =
                [
                    new ActionConfig
                    {
                        Event      = "press",
                        Type       = "process",
                        Executable = "notepad.exe",
                        Arguments  = "",
                    }
                ]
            }
        ];
    }

    [Test]
    public void Dispatch_MatchingPressEvent_ExecutesCommand()
    {
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Press));

        _executor.Received(1).Execute("notepad.exe", "");
    }

    [Test]
    public void Dispatch_UnknownButtonId_DoesNotExecute()
    {
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);
        dispatcher.Dispatch(new InputReport(99, ButtonEventType.Press));

        _executor.DidNotReceive().Execute(Arg.Any<string>(), Arg.Any<string>());
    }

    [Test]
    public void Dispatch_HoldEvent_NoMatchingAction_DoesNotExecute()
    {
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Hold));

        _executor.DidNotReceive().Execute(Arg.Any<string>(), Arg.Any<string>());
    }

    [Test]
    public void Dispatch_MultipleActionsOnSameButton_ExecutesAll()
    {
        _buttons[0].Actions.Add(new ActionConfig
        {
            Event      = "press",
            Type       = "process",
            Executable = "calc.exe",
            Arguments  = "",
        });
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Press));

        _executor.Received(1).Execute("notepad.exe", "");
        _executor.Received(1).Execute("calc.exe",    "");
    }

    [Test]
    public void Dispatch_MatchingEvent_LogsButtonEntry()
    {
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Press));

        var entries = _log.Snapshot();
        Assert.That(entries, Has.Count.EqualTo(1));
        Assert.That(entries[0].Kind,   Is.EqualTo("ボタン"));
        Assert.That(entries[0].Detail, Does.Contain("1"));
        Assert.That(entries[0].Detail, Does.Contain("press"));
    }

    [Test]
    public void Dispatch_WithLabel_IncludesLabelInDetail()
    {
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Press));

        Assert.That(_log.Snapshot()[0].Detail, Does.Contain("Test"));
    }

    [Test]
    public void Dispatch_UnknownButtonId_StillLogsButtonEntry()
    {
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);
        dispatcher.Dispatch(new InputReport(99, ButtonEventType.Press));

        var entries = _log.Snapshot();
        Assert.That(entries, Has.Count.EqualTo(1));
        Assert.That(entries[0].Kind,   Is.EqualTo("ボタン"));
        Assert.That(entries[0].Detail, Does.Contain("99"));
    }

    [Test]
    public void Dispatch_EachEvent_LogsCorrectEventName()
    {
        _buttons[0].Actions.Add(new ActionConfig { Event = "hold",    Type = "process", Executable = "x", Arguments = "" });
        _buttons[0].Actions.Add(new ActionConfig { Event = "release", Type = "process", Executable = "x", Arguments = "" });
        var dispatcher = new CommandDispatcher(_executor, _log, _buttons);

        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Hold));
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Release));

        var entries = _log.Snapshot();
        Assert.That(entries[0].Detail, Does.Contain("hold"));
        Assert.That(entries[1].Detail, Does.Contain("release"));
    }
}
