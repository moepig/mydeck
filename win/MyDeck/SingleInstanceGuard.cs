namespace MyDeck;

/// <summary>
/// 名前付き Mutex でシングルインスタンスを保証し、
/// 2 プロセス目から既存プロセスへシグナルを送る。
/// </summary>
public sealed class SingleInstanceGuard : IDisposable
{
    private readonly Mutex  _mutex;
    private readonly string _eventName;

    public bool IsFirstInstance { get; }

    public SingleInstanceGuard(string mutexName, string eventName)
    {
        _eventName = eventName;
        _mutex     = new Mutex(initiallyOwned: true, name: mutexName, out bool isFirst);
        IsFirstInstance = isFirst;
    }

    /// <summary>
    /// 既存インスタンスが監視している EventWaitHandle にシグナルを送る。
    /// イベントが見つからない場合は false を返す。
    /// </summary>
    public bool TrySignal()
    {
        if (!EventWaitHandle.TryOpenExisting(_eventName, out var ev))
            return false;

        ev.Set();
        ev.Dispose();
        return true;
    }

    public void Dispose()
    {
        if (IsFirstInstance)
            _mutex.ReleaseMutex();
        _mutex.Dispose();
    }
}
