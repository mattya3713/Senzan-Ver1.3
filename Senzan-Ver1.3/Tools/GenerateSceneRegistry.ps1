param(
    [string]$ProjectDir = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ProjectDir)) {
    $ProjectDir = Split-Path -Parent $PSScriptRoot
}

$source_root = Join-Path $ProjectDir "SourceCode"
$scene_root = Join-Path $source_root "Game/00_Scene"
$output_dir = Join-Path $source_root "System/10_Singleton/SceneManager"
$output_header = Join-Path $output_dir "SceneRegistry.generated.h"
$output_source = Join-Path $output_dir "SceneRegistry.generated.cpp"

if (-not (Test-Path $scene_root)) {
    throw "Scene root not found: $scene_root"
}

if (-not (Test-Path $output_dir)) {
    New-Item -ItemType Directory -Path $output_dir -Force | Out-Null
}

$scene_headers = Get-ChildItem -Path $scene_root -Recurse -Filter *.h |
    Where-Object { $_.FullName -notmatch "[\\/]00_Base[\\/]" } |
    Sort-Object FullName

$scene_entries = @()
foreach ($scene_header in $scene_headers) {
    $relative_from_source = [System.IO.Path]::GetRelativePath($source_root, $scene_header.FullName).Replace('\\', '/')
    $class_name = [System.IO.Path]::GetFileNameWithoutExtension($scene_header.Name)

    $enum_name = ($class_name -replace '[^A-Za-z0-9_]', '')
    if ([string]::IsNullOrWhiteSpace($enum_name)) {
        continue
    }

    if ([char]::IsDigit($enum_name[0])) {
        $enum_name = "_" + $enum_name
    }

    $scene_entries += [PSCustomObject]@{
        IncludePath = $relative_from_source
        ClassName = $class_name
        EnumName = $enum_name
    }
}

$scene_entries = $scene_entries | Sort-Object IncludePath -Unique

$header_builder = [System.Text.StringBuilder]::new()
$null = $header_builder.AppendLine("#pragma once")
$null = $header_builder.AppendLine("")
$null = $header_builder.AppendLine("#include <cstddef>")
$null = $header_builder.AppendLine("#include <memory>")
$null = $header_builder.AppendLine("#include <string_view>")
$null = $header_builder.AppendLine("")
$null = $header_builder.AppendLine("class SceneBase;")
$null = $header_builder.AppendLine("")
$null = $header_builder.AppendLine("enum class eList")
$null = $header_builder.AppendLine("{")
foreach ($entry in $scene_entries) {
    $null = $header_builder.AppendLine("    $($entry.EnumName),")
}
$null = $header_builder.AppendLine("    MAX,")
$null = $header_builder.AppendLine("};")
$null = $header_builder.AppendLine("")
$null = $header_builder.AppendLine("std::unique_ptr<SceneBase> CreateSceneById(eList SceneId);")
$null = $header_builder.AppendLine("const char* GetSceneName(eList SceneId);")
$null = $header_builder.AppendLine("std::size_t GetSceneCount();")
$null = $header_builder.AppendLine("eList GetSceneByIndex(std::size_t Index);")
$null = $header_builder.AppendLine("bool TryGetSceneIdByName(std::string_view SceneName, eList& OutSceneId);")

$source_builder = [System.Text.StringBuilder]::new()
$null = $source_builder.AppendLine('#include "SceneRegistry.generated.h"')
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine('#include "Game/00_Scene/00_Base/SceneBase.h"')
foreach ($entry in $scene_entries) {
   $null = $source_builder.AppendLine(('#include "{0}"' -f $entry.IncludePath))
}
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("namespace")
$null = $source_builder.AppendLine("{")
$null = $source_builder.AppendLine("    constexpr eList SCENE_LIST[] =")
$null = $source_builder.AppendLine("    {")
foreach ($entry in $scene_entries) {
    $null = $source_builder.AppendLine("        eList::$($entry.EnumName),")
}
$null = $source_builder.AppendLine("    };")
$null = $source_builder.AppendLine("}")
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("std::unique_ptr<SceneBase> CreateSceneById(eList SceneId)")
$null = $source_builder.AppendLine("{")
$null = $source_builder.AppendLine("    switch (SceneId)")
$null = $source_builder.AppendLine("    {")
foreach ($entry in $scene_entries) {
    $null = $source_builder.AppendLine("    case eList::$($entry.EnumName):")
    $null = $source_builder.AppendLine("        return std::make_unique<$($entry.ClassName)>();")
}
$null = $source_builder.AppendLine("    case eList::MAX:")
$null = $source_builder.AppendLine("    default:")
$null = $source_builder.AppendLine("        return nullptr;")
$null = $source_builder.AppendLine("    }")
$null = $source_builder.AppendLine("}")
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("const char* GetSceneName(eList SceneId)")
$null = $source_builder.AppendLine("{")
$null = $source_builder.AppendLine("    switch (SceneId)")
$null = $source_builder.AppendLine("    {")
foreach ($entry in $scene_entries) {
    $null = $source_builder.AppendLine("    case eList::$($entry.EnumName):")
   $null = $source_builder.AppendLine(('        return "{0}";' -f $entry.EnumName))
}
$null = $source_builder.AppendLine("    case eList::MAX:")
$null = $source_builder.AppendLine("    default:")
$null = $source_builder.AppendLine('        return "Unknown";')
$null = $source_builder.AppendLine("    }")
$null = $source_builder.AppendLine("}")
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("std::size_t GetSceneCount()")
$null = $source_builder.AppendLine("{")
$null = $source_builder.AppendLine("    return sizeof(SCENE_LIST) / sizeof(SCENE_LIST[0]);")
$null = $source_builder.AppendLine("}")
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("eList GetSceneByIndex(std::size_t Index)")
$null = $source_builder.AppendLine("{")
$null = $source_builder.AppendLine("    const std::size_t scene_count = GetSceneCount();")
$null = $source_builder.AppendLine("    if (Index >= scene_count)")
$null = $source_builder.AppendLine("    {")
$null = $source_builder.AppendLine("        return eList::MAX;")
$null = $source_builder.AppendLine("    }")
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("    return SCENE_LIST[Index];")
$null = $source_builder.AppendLine("}")
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("bool TryGetSceneIdByName(std::string_view SceneName, eList& OutSceneId)")
$null = $source_builder.AppendLine("{")
$null = $source_builder.AppendLine("    const std::size_t scene_count = GetSceneCount();")
$null = $source_builder.AppendLine("    for (std::size_t index = 0; index < scene_count; ++index)")
$null = $source_builder.AppendLine("    {")
$null = $source_builder.AppendLine("        const eList scene_id = GetSceneByIndex(index);")
$null = $source_builder.AppendLine("        if (SceneName == GetSceneName(scene_id))")
$null = $source_builder.AppendLine("        {")
$null = $source_builder.AppendLine("            OutSceneId = scene_id;")
$null = $source_builder.AppendLine("            return true;")
$null = $source_builder.AppendLine("        }")
$null = $source_builder.AppendLine("    }")
$null = $source_builder.AppendLine("")
$null = $source_builder.AppendLine("    OutSceneId = eList::MAX;")
$null = $source_builder.AppendLine("    return false;")
$null = $source_builder.AppendLine("}")

Set-Content -Path $output_header -Value $header_builder.ToString() -Encoding UTF8
Set-Content -Path $output_source -Value $source_builder.ToString() -Encoding UTF8
Write-Host "Generated scene registry: $output_header"
Write-Host "Generated scene registry: $output_source"
