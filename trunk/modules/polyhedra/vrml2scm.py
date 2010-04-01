#!/usr/bin/env python
'''
Copyright (C) 2010 Gabor Papp : GPLv3

Convert vrml polyhedra to scheme structs.

Vrml models are downloaded from the Virtual Polyhedra page by George W. Hart
http://www.georgehart.com/virtual-polyhedra/vp.html
'''

import glob, os
import sys

def print_header():
	print '#lang scheme'
	print '(provide (struct-out polyhedron)\n\tpolyhedra)'

	print '(define-struct polyhedron (name points faces edges) #:prefab)\n'
	print '(define polyhedra\n\t#hash('

def print_object(key, name, points, faces, edges):
	print '\t\t(%s .' % key
	print '\t\t\t#s(polyhedron "%s"' % name
	print '\t' * 3,'(',
	for p in points:
		print '#(%s %s %s)' % (p[0], p[1], p[2]),
	print ')\n'
	# faces
	print '\t' * 3,'(',
	for f in faces:
		print '(',
		for v in f:
			print v,
		print ')',
	print ')\n'
	# edges
	print '\t' * 3,'(',
	for e in edges:
		print '(',
		for a in e:
			print a,
		print ')',
	print ')\n'
	# end object
	print '))\n'

def print_footer():
	# end #hash and define
	print '))'

def process(fname):
	print >> sys.stderr, 'processing', fname
	key = os.path.splitext(os.path.basename(fname))[0]
	f = open(fname)
	points = []
	faces = []
	edges = []
	skip = False
	for l in f: 
		# name
		if l.find('Title Info') > -1:
			name = f.next().split('"')[1]
		# points
		elif l.find('Coordinate3') > -1:
			if len(points) != 0: # multiple object sections are not supported
				skip = True
				break
			f.next() # point [
			while 1:
				p = f.next()
				if p.find(']') > -1:
					break
				point = p.split()
				point = map(lambda x: x.rstrip(','), point)
				points.append(point)
		# faces
		elif l.find('IndexedFaceSet') > -1:
			f.next() # coordIndex [
			while 1:
				p = f.next()
				if p.find(']') > -1:
					break
				face = p.strip().split(',')
				faces.append(face[:-2])
		# edges
		elif l.find('IndexedLineSet') > -1:
			f.next() # coordIndex [
			while 1:
				e = f.next()
				if e.find(']') > -1:
					break
				edge = e.strip().split(',')
				edges.append(edge[:-2])
	f.close()
	if not skip:
		print_object(key, name, points, faces, edges)
	else:
		print >> sys.stderr, 'skipped'

def main():
	files = glob.glob('vrml/*.wrl')
	print_header()
	for f in files:
		process(f)
	print_footer()

main()

