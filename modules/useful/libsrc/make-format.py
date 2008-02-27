#!/usr/bin/env python
#
#	Format library generator
#	Copyright (C) 2008 Adam Sampson <ats@offog.org>
#
#	This program is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation, either
#	version 2 of the License, or (at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public
#	License along with this program.  If not, see
#	<http://www.gnu.org/licenses/>.
#

output_header = """--
--	Formatted output library (automatically generated by make-format.py)
--	Copyright (C) 2008 Adam Sampson <ats@offog.org>
--
--	This library is free software; you can redistribute it and/or
--	modify it under the terms of the GNU Lesser General Public
--	License as published by the Free Software Foundation, either
--	version 2 of the License, or (at your option) any later version.
--
--	This library is distributed in the hope that it will be useful,
--	but WITHOUT ANY WARRANTY; without even the implied warranty of
--	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--	Lesser General Public License for more details.
--
--	You should have received a copy of the GNU Lesser General Public
--	License along with this library.  If not, see
--	<http://www.gnu.org/licenses/>.
--

--** @module useful

#USE "format"

"""

class Arg:
	"""A type of argument that you can pass to one of the format PROCs."""

	def formals(self, num):
		return []
	def start(self, w, num):
		pass
	def format(self, w, num):
		pass
	def size(self, num):
		return "0"
	def string(self, num):
		return None
	def finish(self, w, num):
		pass

class ArgNewline(Arg):
	"""No real argument -- just print a newline."""

	def size(self, num):
		return "1"
	def string(self, num):
		return '"*n"'

class ArgString(Arg):
	"""A string."""

	def formals(self, num):
		return ["VAL []BYTE s%d" % num]
	def size(self, num):
		return "(SIZE s%d)" % num
	def string(self, num):
		return "s%d" % num

class ArgInt(Arg):
	"""An integer."""

	def formals(self, num):
		return ["VAL INT i%d" % num]
	def start(self, w, num):
		w.write("[20]BYTE buf%d:" % num)
		w.write("INT start%d:" % num)
	def size(self, num):
		return "((SIZE buf%d) - start%d)" % (num, num)
	def format(self, w, num):
		w.write("format.int (buf%d, start%d, i%d)" % (num, num, num))
	def string(self, num):
		return "[buf%d FROM start%d]" % (num, num)

maximum_args = 5
arg_types = {
	"n": ArgNewline(),
	"s": ArgString(),
	"i": ArgInt(),
	}

class Mode:
	"""An output method for the formatted result."""

	def formals(self):
		return []
	def start(self, w):
		pass
	def pre_output(self, w):
		pass
	def size(self, w, size):
		pass
	def output(self, w, string, size):
		pass
	def post_output(self, w):
		pass
	def finish(self, w):
		pass

class ModePrint(Mode):
	"""Print to a regular channel."""

	name = "print"
	def formals(self):
		return ["CHAN BYTE out!"]
	def output(self, w, string, size):
		w.write("format.print (out!, %s)" % string)

class ModePrintShared(Mode):
	"""Print to a shared channel."""

	name = "prints"
	def formals(self):
		return ["SHARED CHAN BYTE out!"]
	def pre_output(self, w):
		w.write("CLAIM out!")
		w.indent(1)
		w.write("SEQ")
		w.indent(1)
	def output(self, w, string, size):
		w.write("format.print (out!, %s)" % string)
	def post_output(self, w):
		w.indent(-2)

class ModeFormat(Mode):
	"""Format into a string."""

	name = "format"
	def formals(self):
		return ["RESULT MOBILE []BYTE string"]
	def start(self, w):
		w.write("INITIAL INT len IS 0:")
		w.write("INITIAL INT pos IS 0:")
	def size(self, w, size):
		w.write("len := len + %s" % size)
	def pre_output(self, w):
		w.write("string := MOBILE [len]BYTE")
	def output(self, w, string, size):
		w.write("[string FROM pos FOR %s] := %s" % (size, string))
		w.write("pos := pos + %s" % size)

modes = [
	ModePrint(),
	ModePrintShared(),
	ModeFormat(),
	]

class Writer:
	"""Wrapper around a file's write() method that manages indentation."""

	def __init__(self, f):
		self.f = f
		self.level = 0
	def write(self, s):
		self.f.write((" " * (self.level * 2)) + s + "\n")
	def indent(self, n):
		self.level += n

def generate_proc(w, args, mode):
	"""Generate one of the format PROCs."""

	arg_list = [(i, arg_types[args[i]]) for i in range(len(args))]

	formals = []
	for (num, arg) in arg_list:
		formals += arg.formals(num)
	formals += mode.formals()
	w.write("PROC %s.%s (%s)" % (mode.name, "".join(args), ", ".join(formals)))
	w.indent(1)

	for (num, arg) in arg_list:
		arg.start(w, num)
	mode.start(w)
	w.write("SEQ")
	w.indent(1)

	for (num, arg) in arg_list:
		arg.format(w, num)
		mode.size(w, arg.size(num))

	mode.pre_output(w)
	for (num, arg) in arg_list:
		mode.output(w, arg.string(num), arg.size(num))
	mode.post_output(w)

	w.indent(-1)
	for (num, arg) in arg_list:
		arg.finish(w, num)

	mode.finish(w)
	w.indent(-1)
	w.write(":")

def main():
	"""Generate format-gen.occ."""

	f = open("format-gen.occ", "w")
	f.write(output_header)
	w = Writer(f)

	def should_generate(list):
		"""Decide whether a particular set of args should be generated
		(since some aren't very useful.)"""
		if list == []:
			return False
		elif "n" in list[:-1] or list == ["n"]:
			# Newline is only useful at the end.
			return False
		else:
			return True
	def generate(list):
		if should_generate(list):
			for mode in modes:
				generate_proc(w, list, mode)
		if len(list) < maximum_args:
			for a in arg_types.keys():
				generate(list + [a])
	generate([])

	f.close()

if __name__ == "__main__":
	main()