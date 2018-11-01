#!/usr/bin/env python

# based on 'export-sprites.py' and 'glsprite.py' from TCHOW Rainbow; code used is released into the public domain.

# Note: Script meant to be executed from within blender, as per:
# blender --background --python export-meshes.py -- <infile.blend> <outfile.keys> <object>

import sys
import bpy
import struct

args = []
for i in range(0, len(sys.argv)):
    if sys.argv[i] == '--':
        args = sys.argv[i + 1:]

if len(args) != 3:
    print("\n\nUsage:\nblender --background --python export-meshes.py -- <infile.blend> <outfile.keys> <object>\nExports the shapekeys referenced by the object specified (or all if none specified) to a binary blob, indexed by the names of the objects that reference them.\n")
    exit(1)

infile = args[0]
outfile = args[1]
object_to_pull = args[2]

print("Will export object with name " + object_to_pull + " of '" + infile + "' to '" + outfile + "'.")


class FileType:
    def __init__(self, magic, as_lines=False):
        self.magic = magic
        self.position = (b"p" in magic)
        self.as_lines = as_lines
        self.vertex_bytes = 3 * 4


filetypes = {
    ".keys": FileType(b"keys"),
}

filetype = None
for kv in filetypes.items():
    if outfile.endswith(kv[0]):
        assert(filetype is None)
        filetype = kv[1]

if filetype is None:
    print("ERROR: please name outfile with one of:")
    for k in filetypes.keys():
        print("\t\"" + k + "\"")
    exit(1)

bpy.ops.wm.open_mainfile(filepath=infile)

# meshes to write:
to_write = set()
for obj in bpy.data.objects:
    if obj.type == 'MESH' and obj.name == object_to_pull and obj.data.shape_keys:
        to_write_obj = obj
        to_write_mesh = obj.data
        for block in obj.data.shape_keys.key_blocks:  # adds each shape key in
            to_write.add(block)

# data contains vertex position data for each shape key:
data = b''

# strings contains the shapekey and vertex group names:
strings = b''

# index gives offsets into the data (and names) for each shape key:
index = b''

print("Writing Shapekeys...")

vertex_count = 0
for obj in to_write:
    name = obj.name
    print("  " + name)

    # record mesh name, start position and vertex count in the index:
    name_begin = len(strings)
    strings += bytes(name, "utf8")
    name_end = len(strings)
    index += struct.pack('I', name_begin)
    index += struct.pack('I', name_end)

    vgroup_idx = to_write_obj.vertex_groups.find(obj.vertex_group)
    index += struct.pack('I', vgroup_idx + 1)  # add 1 so that 0 => none

    index += struct.pack('I', vertex_count)  # vertex_begin
    # ...count will be written below

    for poly in to_write_mesh.polygons:
        assert(len(poly.loop_indices) == 3)
        for i in range(0, 3):
            assert(to_write_mesh.loops[poly.loop_indices[i]].vertex_index == poly.vertices[i])
            loop = to_write_mesh.loops[poly.loop_indices[i]]
            vertex = obj.data[loop.vertex_index]
            for x in vertex.co:
                data += struct.pack('f', x)

    vertex_count += len(to_write_mesh.polygons) * 3
    index += struct.pack('I', vertex_count)  # vertex_end

# check that we wrote as much data as anticipated:
assert(vertex_count * filetype.vertex_bytes == len(data))

print("...Done.\n\nWriting Vertex Groups...")

# groups contains the vertex group index data
groups = b''
# gindex contains the start/end/string indexes for each vertex group
gindex = b''

gvertex_count = 0

all_groups = [[]] * (len(to_write_obj.vertex_groups))
for v in to_write_mesh.vertices:
    for grp in v.groups:
        all_groups[grp.group].append(v.index)

for grp in to_write_obj.vertex_groups:
    name = grp.name
    print("  " + name)

    name_begin = len(strings)
    strings += bytes(name, "utf8")
    name_end = len(strings)
    gindex += struct.pack('I', name_begin)
    gindex += struct.pack('I', name_end)

    gindex += struct.pack('I', gvertex_count)  # index_begin

    for v in all_groups[grp.index]:
        groups += struct.pack('I', v)

    gvertex_count += len(all_groups[grp.index])
    gindex += struct.pack('I', gvertex_count)  # index_end

print("...Done.\n")

# write the data chunk and index chunk to an output blob:
blob = open(outfile, 'wb')
# first chunk: the data
blob.write(struct.pack('4s', filetype.magic))  # type
blob.write(struct.pack('I', len(data)))  # length
blob.write(data)
# second chunk: the strings
blob.write(struct.pack('4s', b'str0'))  # type
blob.write(struct.pack('I', len(strings)))  # length
blob.write(strings)
# third chunk: the index
blob.write(struct.pack('4s', b'idx0'))  # type
blob.write(struct.pack('I', len(index)))  # length
blob.write(index)
# fourth chunk: vertex groups
blob.write(struct.pack('4s', b'grp0'))  # type
blob.write(struct.pack('I', len(groups)))
blob.write(groups)
# fifth chunk: vertex group indexes
blob.write(struct.pack('4s', b'idx1'))  # type
blob.write(struct.pack('I', len(gindex)))
blob.write(gindex)

wrote = blob.tell()
blob.close()

print("Wrote " + str(wrote) + " bytes [== " + str(len(data)+8) + " bytes of data + " + str(len(strings)+8) + " bytes of strings + " + str(len(index)+8) + " bytes of index + " + str(len(groups)+8) + " bytes of groups + " + str(len(gindex)+8) + " bytes of gindex] to '" + outfile + "'")
