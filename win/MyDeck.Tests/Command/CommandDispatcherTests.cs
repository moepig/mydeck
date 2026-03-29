using MyDeck.Command;
using MyDeck.Config;
using MyDeck.Hid;
using NSubstitute;
using NUnit.Framework;

namespace MyDeck.Tests.Command;

[TestFixture]
public class CommandDispatcherTests
{
    private ICommandExecutor _executor = null!;
    private List<ButtonConfig> _buttons = null!;

    [SetUp]
    public void SetUp()
    {
        _executor = Substitute.For<ICommandExecutor>();
        _buttons =
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
        var dispatcher = new CommandDispatcher(_executor, _buttons);
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Press));

        _executor.Received(1).Execute("notepad.exe", "");
    }

    [Test]
    public void Dispatch_UnknownButtonId_DoesNotExecute()
    {
        var dispatcher = new CommandDispatcher(_executor, _buttons);
        dispatcher.Dispatch(new InputReport(99, ButtonEventType.Press));

        _executor.DidNotReceive().Execute(Arg.Any<string>(), Arg.Any<string>());
    }

    [Test]
    public void Dispatch_HoldEvent_NoMatchingAction_DoesNotExecute()
    {
        var dispatcher = new CommandDispatcher(_executor, _buttons);
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
        var dispatcher = new CommandDispatcher(_executor, _buttons);
        dispatcher.Dispatch(new InputReport(1, ButtonEventType.Press));

        _executor.Received(1).Execute("notepad.exe", "");
        _executor.Received(1).Execute("calc.exe",    "");
    }
}
