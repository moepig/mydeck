namespace MyDeck.Hid;

public interface IHidDevice : IDisposable
{
    bool IsConnected { get; }
    bool TryOpen(ushort vendorId, ushort productId);
    void Close();

    /// <summary>
    /// buffer にデータを読み込む。ブロッキングモードではデータが届くまでスレッドをブロックする。
    /// 戻り値: 読み込んだバイト数（>=1）、-1 = 切断。
    /// </summary>
    int Read(byte[] buffer);

    /// <summary>8バイトの Output Report を送信する。</summary>
    void Write(byte[] data);
}
