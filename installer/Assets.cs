using System.Reflection;

namespace BsvrInstaller;

// Everything that goes into the bsvr mod folder: the embedded payload (including the VR assets that came from
// Half-Life: VR), the converted Blue Shift maps and the player's own files patched for VR.
static class Assets
{
    // Half-Life: VR's HUD PNGs, copied by earlier versions. The game draws only the 640 sheets and the train
    // frames, which are in the payload now; the 320 sheets and the HD/ copies were never drawn.
    static readonly string[] HalfLifeVrHud =
        [.. new[] { "320hud1", "320hud2", "320hud3", "320hud4" }
            .Concat(Enumerable.Range(1, 9).Select(i => $"640hud{i}"))
            .Concat(Enumerable.Range(0, 5).Select(i => $"640_train_frame_{i}"))];

    // Half-Life: VR's skybox sets, copied by earlier versions. The VR code uses only desert, 2desert and alien1, which are
    // in the payload now. hd/ held 1024px copies of all of them that nothing loads.
    static readonly string[] HalfLifeVrSkyboxes =
    [
        "2desert", "alien1", "alien2", "alien3", "black", "city", "cliff", "desert", "dusk", "morning",
        "neb1", "neb2b", "neb6", "neb7", "night", "space", "xen10", "xen8", "xen9",
    ];
    static readonly string[] SkyboxSides = ["rt", "lf", "bk", "ft", "up", "dn"];

    // Files earlier versions installed that are no longer used, removed on update
    static readonly string[] Obsolete =
    [
        // the old VR menu
        "fonts/Roboto-Regular.ttf",
        "textures/null.png",
        "THIRD_PARTY_LICENSES/Nuklear.txt",
        // Half-Life: VR's menu background, only drawn by the removed vr_display_game debug view
        "textures/background.png",
        // HUD PNGs never drawn
        .. HalfLifeVrHud.Take(4).Select(n => $"textures/hud/{n}.png"),
        .. HalfLifeVrHud.Select(n => $"textures/hud/HD/{n}.png"),
        // skyboxes the VR code never shows
        .. HalfLifeVrSkyboxes.Where(n => n is not ("desert" or "2desert" or "alien1"))
            .SelectMany(n => SkyboxSides.Select(side => $"textures/skybox/{n}{side}.png")),
        .. HalfLifeVrSkyboxes.SelectMany(n => SkyboxSides.Select(side => $"textures/skybox/hd/{n}{side}.png")),
        .. SkyboxSides.Select(side => $"textures/skybox/hd/desert{side}_rlt.png"),
    ];

    public static void Install(string halfLifeDir, string modDir)
    {
        Directory.CreateDirectory(modDir);
        RemoveObsolete(modDir);

        Console.WriteLine("Mod files");
        ExtractPayload(modDir, halfLifeDir);

        Console.WriteLine("Maps from Blue Shift");
        Maps.Install(halfLifeDir, modDir);

        Console.WriteLine("Options that do nothing in VR");
        OptionsLocks.Install(halfLifeDir, modDir);

        Console.WriteLine("NPC lines for VR");
        Sentences.Install(halfLifeDir, modDir);

        Console.WriteLine("Saved games");
        RenameSaves(modDir);
    }

    // The old game menu allowed spaces in save names and the new one cannot load those saves, so the spaces
    // are removed. The save and its thumbnail share a name, so every file in SAVE\ is renamed the same way.
    static void RenameSaves(string modDir)
    {
        var saveDir = Path.Combine(modDir, "SAVE");
        if (!Directory.Exists(saveDir))
        {
            return;
        }

        foreach (var file in Directory.GetFiles(saveDir))
        {
            var name = Path.GetFileName(file);
            if (!name.Contains(' '))
            {
                continue;
            }

            var newName = name.Replace(" ", "");
            var target = Path.Combine(saveDir, newName);
            if (File.Exists(target))
            {
                Console.WriteLine($"  warning: {name} not renamed, {newName} already exists");
                continue;
            }
            File.Move(file, target);
            Console.WriteLine($"  {name} -> {newName}");
        }
    }

    // Embedded resource "payload/dlls/hl.dll" goes to "<mod>/dlls/hl.dll", and so on.
    // "halflife/openvr_api.dll" goes next to hl.exe, where Windows resolves client.dll's imports; likewise
    // "halflife/libopenvr_api.so" goes next to hl.sh, which puts that folder on LD_LIBRARY_PATH for client.so.
    // Each set is installed only for the game it is for: native Linux Half-Life has no hl.exe, a Windows one
    // (also under Proton) has no hl_linux.
    // "halflife/bsvr_hd/models/v_357.mdl" goes to the HD game folder next to the mod, "<Half-Life>/bsvr_hd/models/v_357.mdl".
    static void ExtractPayload(string modDir, string halfLifeDir)
    {
        var assembly = Assembly.GetExecutingAssembly();
        var names = assembly.GetManifestResourceNames();
        if (!names.Any(n => n.EndsWith("/hl.dll") || n.EndsWith("/hl.so")) ||
            !names.Any(n => n.EndsWith("/client.dll") || n.EndsWith("/client.so")))
        {
            Console.WriteLine("  warning: game DLLs are not embedded in this build, the mod will not start");
        }

        foreach (var name in names)
        {
            var normalized = name.Replace('\\', '/');
            var isHalfLifeRoot = normalized.StartsWith("halflife/");
            var relative = normalized[(isHalfLifeRoot ? "halflife/".Length : "payload/".Length)..];
            var engine = Path.GetExtension(relative) switch { ".dll" => "hl.exe", ".so" => "hl_linux", _ => null };
            if (isHalfLifeRoot && engine is not null && !File.Exists(Path.Combine(halfLifeDir, engine)))
            {
                continue;
            }
            var target = Path.Combine(isHalfLifeRoot ? halfLifeDir : modDir, relative);
            Console.WriteLine(isHalfLifeRoot ? $"  {relative} (Half-Life folder)" : $"  {relative}");

            Directory.CreateDirectory(Path.GetDirectoryName(target)!);
            using var source = assembly.GetManifestResourceStream(name)!;
            using var destination = File.Create(target);
            source.CopyTo(destination);
        }
    }

    static void RemoveObsolete(string modDir)
    {
        var count = 0;
        foreach (var relative in Obsolete)
        {
            var path = Path.Combine(modDir, relative);
            if (!File.Exists(path))
            {
                continue;
            }
            File.Delete(path);
            count++;

            var dir = Path.GetDirectoryName(path)!;
            if (!Directory.EnumerateFileSystemEntries(dir).Any())
            {
                Directory.Delete(dir);
            }
        }
        if (count > 0)
        {
            Console.WriteLine($"Removed {count} files earlier versions installed that are no longer used");
        }
    }
}
