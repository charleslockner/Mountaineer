
import struct
import binascii

def mat4_string( m ):
    return struct.pack("=16f",
        m[0][0],m[1][0],m[2][0],m[3][0],
        m[0][1],m[1][1],m[2][1],m[3][1],
        m[0][2],m[1][2],m[2][2],m[3][2],
        m[0][3],m[1][3],m[2][3],m[3][3])

def flatten_mat4( m ):
   return m[0]+m[1]+m[2]+m[3]

vertices = [1.0, 1.0, 1.0, 2.0, 2.0, 3.5]
normals = [0.5, 0.5, 0.5, 1.0, 1.0, 1.0];
uvs = [0.0, 0.0, 1.0, 1.0, 0.5, 0.5]
indices = [0, 1, 2, 3, 4]
bones = []
animations = []

# bindPose = [[0.1,0.1,0.1,0.1],[0.1,0.1,0.1,0.1],[0.1,0.1,0.1,0.1],[0.1,0.1,0.1,0.1]]
bindPose = [[0.1,0.2,0.3,0.4],[0.2,0.3,0.4,0.5],[0.3,0.4,0.5,0.6],[0.4,0.5,0.6,0.7]]
parent_index = 0

numChildren = 2
childIndices = [1, 2]

bones.append(struct.pack('=hh%sh16f'%numChildren, parent_index, numChildren, *(childIndices + flatten_mat4(bindPose))))

print binascii.hexlify(bones[0])

numVertices = len(vertices)/3;
numIndices = len(indices);
numBones = len(bones);
numAnimations = len(animations)

buf = ""
buf += struct.pack('=IIii', numVertices, numIndices, numBones, numAnimations)
buf += struct.pack('=b%sf'%len(vertices), 1, *vertices)
buf += struct.pack('=b%sf'%len(normals), 2, *normals)
buf += struct.pack('=b%sf'%len(uvs), 3, *uvs)
buf += struct.pack('=b%sI'%len(indices), 4, *indices)


buf += struct.pack('=b', 8)
for bone in bones:
   buf += bone

# print binascii.hexlify(buf)

