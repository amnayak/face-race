
#!/usr/bin/env python

#based on 'export-sprites.py' and 'glsprite.py' from TCHOW Rainbow; code used is released into the public domain.

#Note: Script meant to be executed from within blender, as per:
#blender --background --python export-meshes.py -- <infile.blend> <outfile.keys> <object>

import sys,re

args = []
for i in range(0,len(sys.argv)):
	if sys.argv[i] == '--':
		args = sys.argv[i+1:]

if len(args) != 3:
	print("\n\nUsage:\nblender --background --python export-meshes.py -- <infile.blend> <outfile.keys> <object>\nExports the shapekeys referenced by the object specified (or all if none specified) to a binary blob, indexed by the names of the objects that reference them.\n")
	exit(1)

infile = args[0]
outfile = args[1]
object_to_pull = args[2]

print("Will export mesh with name " + object_to_pull + " of '" + infile + "' to '" + outfile + "'.")

class FileType:
	def __init__(self, magic, as_lines = False):
		self.magic = magic
		self.position = (b"p" in magic)
		self.as_lines = as_lines
		self.vertex_bytes = 3 * 4

filetypes = {
	".keys" : FileType(b"keys"),
}

filetype = None
for kv in filetypes.items():
	if outfile.endswith(kv[0]):
		assert(filetype == None)
		filetype = kv[1]

if filetype == None:
	print("ERROR: please name outfile with one of:")
	for k in filetypes.keys():
		print("\t\"" + k + "\"")
	exit(1)

import bpy
import struct

import argparse

bpy.ops.wm.open_mainfile(filepath=infile)

#meshes to write:
to_write = set()
for obj in bpy.data.objects:
	if obj.type == 'MESH' and obj.name == object_to_pull and obj.data.shape_keys:
		for block in obj.data.shape_keys.key_blocks: #adds each shape key in
			to_write.add(block)
			to_write_mesh = obj.data

#data contains vertex position data for each shape key:
data = b''

#strings contains the shapekey names:
strings = b''

#index gives offsets into the data (and names) for each mesh:
index = b''

vertex_count = 0
for obj in to_write:
	name = obj.name

	print("Writing '" + name + "'...")

	#record mesh name, start position and vertex count in the index:
	name_begin = len(strings)
	strings += bytes(name, "utf8")
	name_end = len(strings)
	index += struct.pack('I', name_begin)
	index += struct.pack('I', name_end)

	index += struct.pack('I', vertex_count) #vertex_begin
	#...count will be written below

	for poly in to_write_mesh.polygons:
		assert(len(poly.loop_indices) == 3)
		for i in range(0,3):
			assert(to_write_mesh.loops[poly.loop_indices[i]].vertex_index == poly.vertices[i])
			loop = to_write_mesh.loops[poly.loop_indices[i]]
			vertex = obj.data[loop.vertex_index]
			for x in vertex.co:
				data += struct.pack('f', x)

	vertex_count += len(to_write_mesh.polygons) * 3
	index += struct.pack('I', vertex_count) #vertex_end


#check that we wrote as much data as anticipated:
assert(vertex_count * filetype.vertex_bytes == len(data))

#write the data chunk and index chunk to an output blob:
blob = open(outfile, 'wb')
#first chunk: the data
blob.write(struct.pack('4s',filetype.magic)) #type
blob.write(struct.pack('I', len(data))) #length
blob.write(data)
#second chunk: the strings
blob.write(struct.pack('4s',b'str0')) #type
blob.write(struct.pack('I', len(strings))) #length
blob.write(strings)
#third chunk: the index
blob.write(struct.pack('4s',b'idx0')) #type
blob.write(struct.pack('I', len(index))) #length
blob.write(index)
wrote = blob.tell()
blob.close()

print("Wrote " + str(wrote) + " bytes [== " + str(len(data)+8) + " bytes of data + " + str(len(strings)+8) + " bytes of strings + " + str(len(index)+8) + " bytes of index] to '" + outfile + "'")
