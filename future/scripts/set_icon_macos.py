import Cocoa
import sys

Cocoa.NSWorkspace.sharedWorkspace().setIcon_forFile_options_(Cocoa.NSImage.alloc().initWithContentsOfFile_(sys.argv[1].decode('utf-8')), sys.argv[2].decode('utf-8'), 0) or sys.exit("Unable to set file icon")
