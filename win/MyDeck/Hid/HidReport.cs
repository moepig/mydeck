namespace MyDeck.Hid;

public enum ButtonEventType : byte
{
    Press   = 0x01,
    Release = 0x02,
    Hold    = 0x03,
}

public record InputReport(byte ButtonId, ButtonEventType Event)
{
    private const byte ReportIdInput = 0x01;
    private const int  ReportSize    = 8;

    public static InputReport? TryParse(byte[] data)
    {
        if (data.Length < ReportSize) return null;
        if (data[0] != ReportIdInput)  return null;

        byte buttonId = data[1];
        var  ev       = (ButtonEventType)data[2];
        return new InputReport(buttonId, ev);
    }
}

public static class OutputReport
{
    private const byte ReportIdOutput = 0x02;
    private const int  ReportSize     = 8;

    public static byte[] SetLed(bool on)
    {
        var buf = new byte[ReportSize];
        buf[0] = ReportIdOutput;
        buf[1] = 0x01; // SetLed
        buf[2] = (byte)(on ? 1 : 0);
        return buf;
    }
}
