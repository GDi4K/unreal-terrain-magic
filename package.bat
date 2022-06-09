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