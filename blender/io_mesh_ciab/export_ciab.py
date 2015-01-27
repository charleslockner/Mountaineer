import bpy
import bpy_extras
import mathutils

import os
import os.path
import math
import operator
import struct

MAX_INFLUENCES = 3

# #####################################################
# Utils
# #####################################################

def write_file( fname, content ):
    out = open( fname, "wb" )
    out.write( content )
    out.close()

def ensure_folder_exist( foldername ):
    if not os.access( foldername, os.R_OK|os.W_OK|os.X_OK ):
        os.makedirs( foldername )

def ensure_extension( filepath, extension ):
    if not filepath.lower().endswith( extension ):
        filepath += extension
    return filepath

def flatten_mat4( m ):
    return [m[0][0],m[1][0],m[2][0],m[3][0],
            m[0][1],m[1][1],m[2][1],m[3][1],
            m[0][2],m[1][2],m[2][2],m[3][2],
            m[0][3],m[1][3],m[2][3],m[3][3]]

def rToLVec3( v ):
    return mathutils.Vector((v.x, v.z, v.y))

def rToLQuat( q ):
    return mathutils.Quaternion((-q.w, q.x, q.z, q.y))

def rToLMat4( m ):
    nm = m.copy()

    nm[0][1] = m[0][2]
    nm[0][2] = m[0][1]

    nm[1][0] = m[2][0]
    nm[2][0] = m[1][0]

    nm[1][1] = m[2][2]
    nm[2][2] = m[1][1]

    nm[1][2] = m[2][1]
    nm[2][1] = m[1][2]

    nm[3][1] = m[3][2]
    nm[3][2] = m[3][1]

    nm[1][3] = m[2][3]
    nm[2][3] = m[1][3]

    return nm

# def rToLMat4( m ):

# def transpose( m ):
#     n = m.copy()
#     for i in range(0, 4):
#         for j in range(0, 4):
#             n[i][j] = m[j][i]
#     return n

# def swap_xy_mat4( m ):
#     yTemp = m[1].copy()
#     m[1] = m[2].copy()
#     m[2] = yTemp
#     return m

def get_action_state( action, bone, frame ):
    ngroups = len( action.groups )

    pos = mathutils.Vector((0,0,0))
    rot = mathutils.Quaternion((0,0,0,1))
    scl = mathutils.Vector((1,1,1))

    if ngroups > 0:
        index = 0

        for i in range( ngroups ):
            if action.groups[i].name == bone.name:
                index = i

        for channel in action.groups[index].channels:
            value = channel.evaluate( frame )

            if "location" in channel.data_path:
                if channel.array_index == 0:
                    pos.x = value
                elif channel.array_index == 1:
                    pos.y = value
                elif channel.array_index == 2:
                    pos.z = value

            if "quaternion" in channel.data_path:
                if channel.array_index == 0:
                    rot.w = value
                elif channel.array_index == 1:
                    rot.x = value
                elif channel.array_index == 2:
                    rot.y = value
                elif channel.array_index == 3:
                    rot.z = value

            if "scale" in channel.data_path:
                if channel.array_index == 0:
                    scl.x = value
                elif channel.array_index == 1:
                    scl.y = value
                elif channel.array_index == 2:
                    scl.z = value

    pos = bone.head_local + pos
    rot = bone.matrix_local.to_quaternion() * rot
    rot.normalize()

    pos = rToLVec3(pos)
    rot = rToLQuat(rot)

    return pos, rot, scl


def get_animations():
    if( len( bpy.data.armatures ) == 0 ):
        return bytes()

    fps = bpy.data.scenes[0].render.fps
    armature = bpy.data.armatures[0]
    animations = bytes()

    for action in bpy.data.actions:
        end_frame = int( action.frame_range[1] )
        start_frame = int( action.frame_range[0] )
        frame_length = int( end_frame - start_frame )

        # action.name
        animation = struct.pack('=ii', fps, frame_length)

        for bone in armature.bones:
            scale_key_count = rotate_key_count = translate_key_count = frame_length

            translate_keys = struct.pack('=i', translate_key_count)
            rotate_keys = struct.pack('=i', rotate_key_count)
            scale_keys = struct.pack('=i', scale_key_count)

            for frame_number in range( frame_length ):
                pos, rot, scl = get_action_state( action, bone, frame_number )

                px, py, pz = pos.x, pos.y, pos.z
                rw, rx, ry, rz = rot.w, rot.x, rot.y, rot.z
                sx, sy, sz = scl.x, scl.y, scl.z

                translate_keys += struct.pack('=i3f', frame_number, px, py, pz)
                rotate_keys += struct.pack('=i4f', frame_number, rw, rx, ry, rz)
                scale_keys += struct.pack('=i3f', frame_number, sx, sy, sz)

            animation += translate_keys + rotate_keys + scale_keys

        animations += animation

    return animations

def get_mesh_data( obj ):
    mesh = obj.to_mesh( bpy.context.scene, True, "PREVIEW" )

    vertices = []
    normals = []
    colors = []
    uvs = []
    indices = []
    bones = []
    boneIndices = []
    boneWeights = []

    vertex_number = -1
    for face in obj.data.polygons:
        vertices_in_face = face.vertices[:]

        for vertex_index in vertices_in_face:

            vertex_number += 1

            v_co = rToLVec3(obj.data.vertices[vertex_index].co)
            vertices.append( v_co.x )
            vertices.append( v_co.y )
            vertices.append( v_co.z )

            v_normal = rToLVec3(obj.data.vertices[vertex_index].normal)
            normals.append( v_normal.x )
            normals.append( v_normal.y )
            normals.append( v_normal.z )

            indices.append( vertex_number )

    if len( mesh.tessface_uv_textures ) > 0:
        for data in mesh.tessface_uv_textures.active.data:
            uvs.append( data.uv1.x )
            uvs.append( data.uv1.y )
            uvs.append( data.uv2.x )
            uvs.append( data.uv2.y )
            uvs.append( data.uv3.x )
            uvs.append( data.uv3.y )

    if len( mesh.tessface_vertex_colors ) > 0:
        for data in mesh.tessface_vertex_colors.active.data:
            colors.append( data.color1.r )
            colors.append( data.color1.g )
            colors.append( data.color1.b )
            colors.append( data.color2.r )
            colors.append( data.color2.g )
            colors.append( data.color2.b )
            colors.append( data.color3.r )
            colors.append( data.color3.g )
            colors.append( data.color3.b )

    if( len( bpy.data.armatures ) > 0 ):
        armature = bpy.data.armatures[0]

        for face in obj.data.polygons:
            vertices_in_face = face.vertices[:]

            for vertex_index in vertices_in_face:
                vertex = obj.data.vertices[ vertex_index ]
                bone_array = []

                for group in vertex.groups:
                    index = group.group
                    weight = group.weight

                    bone_array.append( (index, weight) )

                for i in range( MAX_INFLUENCES ):

                    if i < len( bone_array ):
                        bone_proxy = bone_array[i]

                        found = 0
                        index = bone_proxy[0]
                        weight = bone_proxy[1]

                        for j, bone in enumerate( armature.bones ):
                            if obj.vertex_groups[ index ].name == bone.name:
                                boneIndices.append(j)
                                boneWeights.append(weight)
                                found = 1
                                break

                        if found != 1:
                            boneIndices.append(0)
                            boneWeights.append(0)
                    else:
                        boneIndices.append(0)
                        boneWeights.append(0)

        bone_id = -1
        for bone in armature.bones:
            bone_id += 1

            parent_index = -1
            childIndices = []
            weight = 0
            skinned = "false"
            inheritRotation = "false"
            inheritScale = "false"

            pos = bone.head

            if bone.parent != None:
                parent_index = i = 0
                pos = bone.head_local - bone.parent.head_local

                for parent in armature.bones:
                    if parent.name == bone.parent.name:
                        parent_index = i
                    i += 1

            if bone.children != None:
                child_index = 0

                for possible_child in armature.bones:
                    for child in bone.children:
                        if possible_child.name == child.name:
                            childIndices.append(child_index)
                            break
                    child_index += 1

            j = -1
            for boneIndex in boneIndices:
                j += 1
                if int( boneIndex ) == bone_id:
                    weight += float( boneWeights[j] )

            # name = bone.name
            bindPose = rToLMat4(bone.matrix_local.inverted())

            numChildren = len(childIndices)

            bones.append(struct.pack('=hh%sh16f'%numChildren, parent_index, numChildren, *(childIndices+flatten_mat4(bindPose))))

    numVertices = int(len(vertices)/3)
    numIndices = len(indices)
    numBones = len(bones)
    numAnimations = len(bpy.data.actions)

    buf  = struct.pack('=iiii', numVertices, numIndices, numBones, numAnimations)
    buf += struct.pack('=b%sf'%len(vertices), 1, *vertices)
    buf += struct.pack('=b%sf'%len(normals), 2, *normals)

    if len(colors) > 0:
        buf += struct.pack('=b%sf'%len(colors), 3, *colors)
    if len(uvs) > 0:
        buf += struct.pack('=b%sf'%len(uvs), 4, *uvs)

    buf += struct.pack('=b%sI'%len(indices), 5, *indices)

    if len(bpy.data.armatures) > 0:
        buf += struct.pack('=b%sI'%len(boneIndices), 6, *boneIndices)
        buf += struct.pack('=b%sf'%len(boneWeights), 7, *boneWeights)
        buf += struct.pack('=b', 8)
        for bone_buf in bones:
            buf += bone_buf

    if len(bpy.data.actions) > 0:
        buf += struct.pack('=b', 9)
        buf += get_animations()

    return buf

def export_mesh( obj, filepath ):

    # axisSwapM = bpy_extras.io_utils.axis_conversion(from_forward='Y', from_up='Z', to_forward='Y', to_up='Z')

    buf = get_mesh_data( obj )
    write_file( filepath, buf )
    print("writing", filepath, "done")


# #####################################################
# Main
# #####################################################

def save( operator, context, filepath = "" ):
    filepath = ensure_extension( filepath, ".ciab")

    bpy.ops.object.duplicate()
    bpy.ops.object.mode_set( mode = "OBJECT" )
    bpy.ops.object.modifier_add( type="TRIANGULATE" )
    bpy.ops.object.modifier_apply( apply_as = "DATA", modifier = "Triangulate" )

    export_mesh( context.active_object, filepath )
    bpy.ops.object.delete()

    return {"FINISHED"}
