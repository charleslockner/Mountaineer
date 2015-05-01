
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>

// #include "safe_gl.h"
// #include "model.h"

// typedef enum {
//    POSITIONS = 1,
//    NORMALS = 2,
//    COLORS = 3,
//    TEXCOORDS = 4,
//    TANGENTS = 5,
//    BITANGENTS = 6,
//    INDICES = 7,
//    BONE_INDICES = 8,
//    BONE_WEIGHTS = 9,
//    BONE_NUM_INF = 10,
//    BONE_TREE = 11,
//    ANIMATIONS = 12
// } modelFieldType;

// static void readHeader(FILE *fp, Model * model) {
//    fread(& model->vertexCount, sizeof(unsigned int), 1, fp);
//    fread(& model->faceCount, sizeof(unsigned int), 1, fp);
//    fread(& model->boneCount, sizeof(unsigned int), 1, fp);
//    fread(& model->animationCount, sizeof(unsigned int), 1, fp);

//    // printf("verts: %d, faces: %d, bones: %d, anims: %d\n",
//    //    model->vertexCount, model->faceCount, model->boneCount, model->animationCount);

//    if (model->boneCount > MAX_BONES) {
//       printf("There are %d bones and the max is %d\n", model->boneCount, MAX_BONES);
//       exit(1);
//    }

//    model->vertices = std::vector<Vertex>(model->vertexCount);
//    // printf("numVertices = %d, size = %d\n", model->vertexCount, model->vertices.size());
//    model->faces = std::vector<Face>(model->faceCount);
// }

// static unsigned int createVBO(FILE *fp, int count, int size) {
//    void * data = (void *)malloc(count * size);
//    fread(data, size, count, fp);

//    unsigned int vbo;
//    glGenBuffers(1, & vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBufferData(GL_ARRAY_BUFFER, count * size, data, GL_STATIC_DRAW);

//    free(data);

//    return vbo;
// }

// static void readPositions(FILE *fp, Model * model) {
//    int count = 3 * model->vertexCount;
//    int size = sizeof(float);

//    float * data = (float *)malloc(count * size);
//    fread(data, size, count, fp);

//    for (int i = 0; i < model->vertexCount; i++)
//       model->vertices[i].position = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

//    unsigned int vbo;
//    glGenBuffers(1, & vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBufferData(GL_ARRAY_BUFFER, count * size, data, GL_STATIC_DRAW);

//    free(data);

//    model->posID = vbo;
// }

// static void readNormals(FILE *fp, Model * model) {
//    int count = 3 * model->vertexCount;
//    int size = sizeof(float);

//    float * data = (float *)malloc(count * size);
//    fread(data, size, count, fp);

//    for (int i = 0; i < model->vertexCount; i++)
//       model->vertices[i].normal = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

//    unsigned int vbo;
//    glGenBuffers(1, & vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBufferData(GL_ARRAY_BUFFER, count * size, data, GL_STATIC_DRAW);

//    free(data);

//    model->normID = vbo;
// }

// static void readColors(FILE *fp, Model * model) {
//    model->colorID = createVBO(fp, 3 * model->vertexCount, sizeof(float));
// }

// static void readTexCoords(FILE *fp, Model * model) {
//    model->uvID = createVBO(fp, 2 * model->vertexCount, sizeof(float));
// }

// static void readTangents(FILE *fp, Model * model) {
//    int count = 3 * model->vertexCount;
//    int size = sizeof(float);

//    float * data = (float *)malloc(count * size);
//    fread(data, size, count, fp);

//    for (int i = 0; i < model->vertexCount; i++)
//       model->vertices[i].tangent = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

//    unsigned int vbo;
//    glGenBuffers(1, & vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBufferData(GL_ARRAY_BUFFER, count * size, data, GL_STATIC_DRAW);

//    free(data);

//    model->tanID = vbo;
// }

// static void readBitangents(FILE *fp, Model * model) {
//    int count = 3 * model->vertexCount;
//    int size = sizeof(float);

//    float * data = (float *)malloc(count * size);
//    fread(data, size, count, fp);

//    for (int i = 0; i < model->vertexCount; i++)
//       model->vertices[i].bitangent = Eigen::Vector3f(data[3*i], data[3*i+1], data[3*i+2]);

//    unsigned int vbo;
//    glGenBuffers(1, & vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glBufferData(GL_ARRAY_BUFFER, count * size, data, GL_STATIC_DRAW);

//    free(data);

//    model->bitanID = vbo;
// }

// static void readIndices(FILE *fp, Model * model) {
//    model->indID = createVBO(fp, 3 * model->faceCount, sizeof(unsigned int));
// }

// static void readBoneIndices(FILE *fp, Model * model) {
//    int count = MAX_INFLUENCES * model->vertexCount;

//    unsigned short * ushortData = (unsigned short *)malloc(count * sizeof(unsigned short));
//    fread(ushortData, sizeof(unsigned short), count, fp);

//    // convert the data from unsigned shorts to floats
//    float * floatData = (float *)malloc(count * sizeof(float));
//    for (int i = 0; i < count; i++)
//       floatData[i] = ushortData[i];

//    // generate the buffer
//    glGenBuffers(1, & model->bIndID);
//    glBindBuffer(GL_ARRAY_BUFFER, model->bIndID);
//    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), floatData, GL_STATIC_DRAW);

//    // free the data blocks
//    free(ushortData);
//    free(floatData);
// }

// static void readBoneWeights(FILE *fp, Model * model) {
//    model->bWeightID = createVBO(fp, MAX_INFLUENCES * model->vertexCount, sizeof(float));
// }

// static void readBoneInfluences(FILE *fp, Model * model) {
//    int count = model->vertexCount;

//    unsigned short * ushortData = (unsigned short *)malloc(count * sizeof(unsigned short));
//    fread(ushortData, sizeof(unsigned short), count, fp);

//    // convert the data from unsigned shorts to floats
//    float * floatData = (float *)malloc(count * sizeof(float));
//    for (int i = 0; i < count; i++)
//       floatData[i] = ushortData[i];

//    // generate the buffer
//    glGenBuffers(1, & model->bNumInfID);
//    glBindBuffer(GL_ARRAY_BUFFER, model->bNumInfID);
//    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), floatData, GL_STATIC_DRAW);

//    // free the data blocks
//    free(ushortData);
//    free(floatData);
// }

// static void readBoneTree(FILE *fp, Model * model) {
//    fread(& model->boneRoot, sizeof(short), 1, fp);

//    model->bones = std::vector<Bone>(model->boneCount);

//    for (int i = 0; i < model->boneCount; i++) {
//       Bone * bone = & model->bones[i];
//       fread(& bone->parentIndex, sizeof(short), 1, fp);
//       fread(& bone->childCount, sizeof(short), 1, fp);

//       bone->childIndices = std::vector<short>(bone->childCount);
//       for (int j = 0; j < bone->childCount; j++)
//          fread(& bone->childIndices[j], sizeof(short), 1, fp);

//       fread(& bone->invBonePose, sizeof(Eigen::Matrix4f), 1, fp);
//       fread(& bone->parentOffset, sizeof(Eigen::Matrix4f), 1, fp);
//    }
// }

// static void readAnimations(FILE *fp, Model * model) {
//    model->animations = std::vector<Animation>(model->animationCount);

//    for (int i = 0; i < model->animationCount; i++) {
//       Animation * anim = & model->animations[i];

//       fread(& anim->fps, sizeof(unsigned int), 1, fp);
//       fread(& anim->keyCount, sizeof(unsigned int), 1, fp);
//       anim->duration = 1.0 * (anim->keyCount-1) / anim->fps;

//       anim->animBones = std::vector<AnimBone>(model->boneCount);
//       for (int j = 0; j < model->boneCount; j++) {
//          AnimBone * animBone = & anim->animBones[j];

//          animBone->keys = std::vector<Key>(anim->keyCount);
//          for (int k = 0; k < anim->keyCount; k++) {
//             Key * key = & animBone->keys[k];

//             fread(& key->time, sizeof(float), 1, fp);
//             fread(& key->position, sizeof(float), 3, fp);
//             fread(& key->rotation, sizeof(float), 4, fp);
//             fread(& key->scale, sizeof(float), 3, fp);
//          }
//       }
//    }
// }

// static void loadMeshData(FILE *fp, Model * model) {
//    readHeader(fp, model);
//    readVertices(fp, model);
//    readIndices(fp, model);
//    readBoneTree(fp, model);
//    readAnimations(fp, model);

//    checkOpenGLError();
// }

// static FILE * safe_fopen(const char * path) {
//    if (!path) {
//       fprintf(stderr, "Error reading model file. Filename string empty\n");
//       exit(1);
//    }

//    FILE * fp = fopen(path,"rb");

//    if (!fp) {
//       printf("Error loading %s:\n ", path);
//       perror("");
//       exit(1);
//    }

//    return fp;
// }

// void Model::loadVBV(const char * path) {
//    FILE * fp = safe_fopen(path);
//    loadMeshData(fp, this);
//    fclose(fp);

//    // printBoneTree();
//    // printAnimations();

//    fprintf(stderr, "Loaded CIAB model: %s\n", path);
// }
