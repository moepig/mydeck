using System.Text.Json;

namespace MyDeck.Config;

public static class ConfigLoader
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNameCaseInsensitive = true,
        ReadCommentHandling         = JsonCommentHandling.Skip,
    };

    public static AppConfig Load(string path)
    {
        var json   = File.ReadAllText(path);
        var config = JsonSerializer.Deserialize<AppConfig>(json, JsonOptions)
                     ?? throw new InvalidDataException("設定ファイルのデシリアライズに失敗しました。");
        Validate(config);
        return config;
    }

    private static readonly HashSet<string> ValidEvents = ["press", "hold", "release"];

    private static readonly JsonSerializerOptions SaveOptions = new()
    {
        WriteIndented        = true,
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
    };

    public static void Save(AppConfig config, string path)
    {
        var dir = Path.GetDirectoryName(path);
        if (!string.IsNullOrEmpty(dir))
            Directory.CreateDirectory(dir);

        File.WriteAllText(path, JsonSerializer.Serialize(config, SaveOptions));
    }

    private static void Validate(AppConfig config)
    {
        var ids = new HashSet<byte>();
        foreach (var btn in config.Buttons)
        {
            if (!ids.Add(btn.ButtonId))
                throw new InvalidDataException($"buttonId {btn.ButtonId} が重複しています。");

            foreach (var action in btn.Actions)
            {
                if (!ValidEvents.Contains(action.Event))
                    throw new InvalidDataException(
                        $"ボタン {btn.ButtonId}: 不正な event 値 '{action.Event}'。");
            }
        }
    }
}
