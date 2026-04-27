$binFile = "C:\workspace\libs\VulkanSDK\1.4.341.0\Bin\glslangValidator.exe"
$srcPath = "C:\workspace\code\PhotonRenderer\src\Shaders\GLSL"
$dstPath = "C:\workspace\code\PhotonRenderer\cache\shaders\spv"
Get-ChildItem $srcPath -File -Name -Recurse |
ForEach-Object {
	$fn = $_.split('\')[-1]
    $srcFile = "$srcPath\$_"
	$dstFile = "$dstPath\$fn.spv"
    $parts = $_.Split(".")
    $shader = $parts[0]
    $ext = $parts[-1]
    if($ext -eq "vert" -or $ext -eq "frag" -or $ext -eq "geom" -or $ext -eq "comp")
    {
        & $binFile -V $srcFile -o $dstFile
    }
}