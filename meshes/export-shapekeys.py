
#!/usr/bin/env python

#based on 'export-sprites.py' and 'glsprite.py' from TCHOW Rainbow; code used is released into the public domain.

#Note: Script meant to be executed from within blender, as per:
#blender --background --python export-meshes.py -- <infile.blend>[:layer] <outfile.p[n][c][t]>

import sys,re

args = []
for i in range(0,len(sys.argv)):
	if sys.argv[i] == '--':
		args = sys.argv[i+1:]

if len(args) != 2:
	print("\n\nUsage:\nblender --background --python export-meshes.py -- <infile.blend>[:layer] <outfile.p[n][c][t][l]>\nExports the meshes referenced by all objects in layer (default 1) to a binary blob, indexed by the names of the objects that reference them. If 'l' is specified in the file extension, only mesh edges will be exported.\n")
	exit(1)

infile = args[0]
layer = 1
m = re.match(r'^(.*):(\d+)$', infile)
if m:
	infile = m.group(1)
	layer = int(m.group(2))
outfile = args[1]

assert layer >= 1 and layer <= 20

print("Will export meshes referenced from layer " + str(layer) + " of '" + infile + "' to '" + outfile + "'.")

class FileType:
	def __init__(self, magic, as_lines = False):
		self.magic = magic
		self.position = (b"p" in magic)
		self.as_lines = as_lines
		self.vertex_bytes = 3 * 4

filetypes = {
	".p" : FileType(b"p..."),
	".pl" : FileType(b"p...", True),
	".pn" : FileType(b"pn.."),
	".pc" : FileType(b"pc.."),
	".pt" : FileType(b"pt.."),
	".pnc" : FileType(b"pnc."),
	".pct" : FileType(b"pct."),
	".pnt" : FileType(b"pnt."),
	".pnct" : FileType(b"pnct"),
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
	if obj.layers[layer-1] and obj.type == 'MESH' and obj.data.shape_keys:
		for block in obj.data.shape_keys.key_blocks: #adds each shape key in
			to_write.add(block)

#data contains vertex and normal data from the meshes:
data = b''

#strings contains the mesh names:
strings = b''

#index gives offsets into the data (and names) for each mesh:
index = b''

vertex_count = 0
for obj in to_write:

	mesh = obj
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

	for d in obj.data:
		vertex = d.co
		data += struct.pack('fff', *vertex)
		vertex_count += 1;

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
