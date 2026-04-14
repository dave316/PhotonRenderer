$binFile = "C:\workspace\libs\VulkanSDK\1.4.328.1\Bin\glslangValidator.exe"
$path = "C:\workspace\code\PhotonRenderer\src\Shaders\GLSL"
Get-ChildItem $path -File -Name -Recurse |
ForEach-Object {
	#echo $_
    $name = "$path\$_"
    $parts = $_.Split(".")
    $shader = $parts[0]
    $ext = $parts[-1]
    if($ext -eq "vert" -or $ext -eq "frag" -or $ext -eq "geom" -or $ext -eq "comp")
    {
        & $binFile -V $name -o "$name.spv"
    }
}