using HidApi;

namespace MyDeck.Hid;

public sealed class HidDevice : IHidDevice
{
    private Device? _device;

    public bool IsConnected => _device is not null;

    public bool TryOpen(ushort vendorId, ushort productId)
    {
        try
        {
            _device = new Device(vendorId, productId);
            // ブロッキングモード：データが届くまで Read() でスレッドを待機させる
            _device.SetNonBlocking(false);
            return true;
        }
        catch
        {
            _device = null;
            return false;
        }
    }

    public void Close()
    {
        _device?.Dispose();
        _device = null;
    }

    /// <inheritdoc/>
    public int Read(byte[] buffer)
    {
        if (_device is null) return -1;
        try
        {
            var span = _device.Read(buffer.Length);
            if (span.Length == 0) return 0;
            span.CopyTo(buffer);  // 呼び出し元のバッファへコピー（アロケーションなし）
            return span.Length;
        }
        catch
        {
            Close();
            return -1;
        }
    }

    public void Write(byte[] data)
    {
        _device?.Write(data);
    }

    public void Dispose() => Close();
}
