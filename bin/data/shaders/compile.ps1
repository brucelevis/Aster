$sourcePath = "glsl/"

$compilerExe = "glslangValidator.exe"

foreach($srcFile in (Get-ChildItem $sourcePath -Recurse | Where-Object Extension -In '.frag','.vert','.comp')) {
  $srcPath = $srcFile.fullName;
  $srcName = $srcFile.Name
  $dstPath = $srcPath.Replace("\glsl\", "\spirv\") + ".spv";
  $dstFolder = Split-Path $dstPath
  
  if(!(Test-Path $dstFolder))
  {
    New-Item -path $dstFolder -type directory
  }
  
  Invoke-Expression "$compilerExe -V `"$srcPath`" -l --target-env vulkan1.2 -o `"$dstPath`""
}