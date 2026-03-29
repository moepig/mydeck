namespace MyDeck.Config;

public record AppConfig
{
    public int          Version { get; init; } = 1;
    public DeviceConfig Device  { get; init; } = new();
    public List<ButtonConfig> Buttons { get; init; } = [];
}

public record DeviceConfig
{
    public ushort VendorId             { get; init; } = 0x1B4F;
    public ushort ProductId            { get; init; } = 0x9206;
    public int    PollingIntervalMs    { get; init; } = 100;
    public int    ReconnectIntervalMs  { get; init; } = 5000;
}

public record ButtonConfig
{
    public byte   ButtonId { get; init; }
    public string Label    { get; init; } = string.Empty;
    public List<ActionConfig> Actions { get; init; } = [];
}

public record ActionConfig
{
    public string Event      { get; init; } = "press"; // press | hold | release
    public string Type       { get; init; } = "process";
    public string Executable { get; init; } = string.Empty;
    public string Arguments  { get; init; } = string.Empty;
}
