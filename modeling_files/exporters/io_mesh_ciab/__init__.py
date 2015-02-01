bl_info = {
    "name":         "CIAB Mesh Export",
    "author":       "Charles Lockner",
    "blender":      (2,7,2),
    "version":      (0,0,0),
    "location":     "File > Import-Export",
    "description":  "Import-Export CIAB data format. CIAB is a binary data format"
                    "that holds information for vertex positions, normals, uvCoords,"
                    "face indices, bones, and animations.",
    "category":     "Import-Export",
    # "wiki_url":     "https://github.com/clockner",
    # "tracker_url":  "https://github.com/clockner",
}

import bpy
from bpy.props import *
from bpy_extras.io_utils import ExportHelper, ImportHelper


# ################################################################
# Import CIAB
# ################################################################

class ImportCIAB( bpy.types.Operator, ImportHelper ):
    bl_idname = "import.ciab"
    bl_label = "Import CIAB"

    filename_ext = ".ciab"
    filter_glob = StringProperty( default="*.ciab", options={"HIDDEN"})

    def execute( self, context ):
        import io_mesh_ciab.import_ciab
        return io_mesh_ciab.import_ciab.load( self, context, **self.properties )


# ################################################################
# Export CIAB
# ################################################################

class ExportCIAB( bpy.types.Operator, ExportHelper ):
    bl_idname = "export.ciab"
    bl_label = "Export CIAB"

    filename_ext = ".ciab"

    def invoke( self, context, event ):
        return ExportHelper.invoke( self, context, event )

    @classmethod
    def poll( cls, context ):
        return context.active_object != None

    def execute( self, context ):
        print("Selected: " + context.active_object.name )

        if not self.properties.filepath:
            raise Exception("filename not set")

        filepath = self.filepath

        import io_mesh_ciab.export_ciab
        return io_mesh_ciab.export_ciab.save( self, context, **self.properties )


# ################################################################
# Common
# ################################################################

def menu_func_export( self, context ):
    default_path = bpy.data.filepath.replace(".blend", ".ciab")
    self.layout.operator( ExportCIAB.bl_idname, text="CIAB (.ciab)").filepath = default_path

def menu_func_import( self, context ):
    self.layout.operator( ImportCIAB.bl_idname, text="CIAB (.ciab)")

def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(menu_func_export)
    bpy.types.INFO_MT_file_import.append(menu_func_import)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)
    bpy.types.INFO_MT_file_import.remove(menu_func_import)

if __name__ == "__main__":
    register()