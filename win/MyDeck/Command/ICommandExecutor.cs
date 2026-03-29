namespace MyDeck.Command;

public interface ICommandExecutor
{
    void Execute(string executable, string arguments);
}
