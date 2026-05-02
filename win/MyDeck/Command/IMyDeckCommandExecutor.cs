namespace MyDeck.Command;

public interface IMyDeckCommandExecutor
{
    void Execute(string executable, string arguments);
}
