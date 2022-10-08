echo "Packaging ..."

rmdir /S /Q ToPackage

@REM --- OpenLand ---
md ToPackage
md ToPackage\TerrainMagic

copy TerrainMagicDevApp\Plugins\TerrainMagic\TerrainMagic.uplugin ToPackage\TerrainMagic\TerrainMagic.uplugin /Y

md ToPackage\TerrainMagic\Config
xcopy TerrainMagicDevApp\Plugins\TerrainMagic\Config ToPackage\TerrainMagic\Config /E/H

md ToPackage\TerrainMagic\Content
xcopy TerrainMagicDevApp\Plugins\TerrainMagic\Content ToPackage\TerrainMagic\Content /E/H

md ToPackage\TerrainMagic\Source
xcopy TerrainMagicDevApp\Plugins\TerrainMagic\Source ToPackage\TerrainMagic\Source /E/H

md ToPackage\TerrainMagic\Resources
xcopy TerrainMagicDevApp\Plugins\TerrainMagic\Resources ToPackage\TerrainMagic\Resources /E/H

echo "Copying the UE5 version"
md ToPackage\TerrainMagic_UE5
xcopy ToPackage\TerrainMagic ToPackage\TerrainMagic_UE5 /E/H

cscript //NoLogo sed.vbs s/(4.27.0)/5.0.0/ < ToPackage\TerrainMagic\TerrainMagic.uplugin > ToPackage\TerrainMagic_UE5\TerrainMagic.uplugin

echo "Copying the UE5.1 version"
md ToPackage\TerrainMagic_UE51
xcopy ToPackage\TerrainMagic ToPackage\TerrainMagic_UE51 /E/H

cscript //NoLogo sed.vbs s/(4.27.0)/5.1.0/ < ToPackage\TerrainMagic\TerrainMagic.uplugin > ToPackage\TerrainMagic_UE51\TerrainMagic.uplugin

@REM Identifying the version
FOR /F "tokens=*" %%g IN ('git describe --tags') do (SET VERSION=%%g)

@REM Making zip files
cd ToPackage
tar.exe -a -c -f TerrainMagic_UE51_%VERSION%.zip TerrainMagic_UE51
tar.exe -a -c -f TerrainMagic_UE5_%VERSION%.zip TerrainMagic_UE5
tar.exe -a -c -f TerrainMagic_%VERSION%.zip TerrainMagic
