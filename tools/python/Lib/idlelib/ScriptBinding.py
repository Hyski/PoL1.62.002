"""Extension to execute code outside the Python shell window.

This adds the following commands:

- Check module does a full syntax check of the current module.
  It also runs the tabnanny to catch any inconsistent tabs.

- Run module executes the module's code in the __main__ namespace.  The window
  must have been saved previously. The module is added to sys.modules, and is
  also added to the __main__ namespace.

XXX GvR Redesign this interface (yet again) as follows:

- Present a dialog box for ``Run Module''

- Allow specify command line arguments in the dialog box

"""

import os
import re
import string
import tabnanny
import tokenize
import tkMessageBox
import PyShell

from configHandler import idleConf

IDENTCHARS = string.ascii_letters + string.digits + "_"

indent_message = """Error: Inconsistent indentation detected!

This means that either:

1) your indentation is outright incorrect (easy to fix), or

2) your indentation mixes tabs and spaces in a way that depends on \
how many spaces a tab is worth.

To fix case 2, change all tabs to spaces by using Select All followed \
by Untabify Region (both in the Edit menu)."""


class ScriptBinding:

    menudefs = [
        ('run', [None,
                 ('Check Module', '<<check-module>>'),
                 ('Run Module', '<<run-module>>'), ]), ]

    def __init__(self, editwin):
        self.editwin = editwin
        # Provide instance variables referenced by Debugger
        # XXX This should be done differently
        self.flist = self.editwin.flist
        self.root = self.flist.root

    def check_module_event(self, event):
        filename = self.getfilename()
        if not filename:
            return
        if not self.tabnanny(filename):
            return
        self.checksyntax(filename)

    def tabnanny(self, filename):
        f = open(filename, 'r')
        try:
            tabnanny.process_tokens(tokenize.generate_tokens(f.readline))
        except tokenize.TokenError, msg:
            msgtxt, (lineno, start) = msg
            self.editwin.gotoline(lineno)
            self.errorbox("Tabnanny Tokenizing Error",
                          "Token Error: %s" % msgtxt)
            return False
        except tabnanny.NannyNag, nag:
            # The error messages from tabnanny are too confusing...
            self.editwin.gotoline(nag.get_lineno())
            self.errorbox("Tab/space error", indent_message)
            return False
        return True

    def checksyntax(self, filename):
        f = open(filename, 'r')
        source = f.read()
        f.close()
        if '\r' in source:
            source = re.sub(r"\r\n", "\n", source)
        if source and source[-1] != '\n':
            source = source + '\n'
        text = self.editwin.text
        text.tag_remove("ERROR", "1.0", "end")
        try:
            # If successful, return the compiled code
            return compile(source, filename, "exec")
        except (SyntaxError, OverflowError), err:
            try:
                msg, (errorfilename, lineno, offset, line) = err
                if not errorfilename:
                    err.args = msg, (filename, lineno, offset, line)
                    err.filename = filename
                self.colorize_syntax_error(msg, lineno, offset)
            except:
                msg = "*** " + str(err)
            self.errorbox("Syntax error",
                          "There's an error in your program:\n" + msg)
            return False

    def colorize_syntax_error(self, msg, lineno, offset):
        text = self.editwin.text
        pos = "0.0 + %d lines + %d chars" % (lineno-1, offset-1)
        text.tag_add("ERROR", pos)
        char = text.get(pos)
        if char and char in IDENTCHARS:
            text.tag_add("ERROR", pos + " wordstart", pos)
        if '\n' == text.get(pos):   # error at line end
            text.mark_set("insert", pos)
        else:
            text.mark_set("insert", pos + "+1c")
        text.see(pos)

    def run_module_event(self, event):
        """Run the module after setting up the environment.

        First check the syntax.  If OK, make sure the shell is active and
        then transfer the arguments, set the run environment's working
        directory to the directory of the module being executed and also
        add that directory to its sys.path if not already included.

        """
        filename = self.getfilename()
        if not filename:
            return
        code = self.checksyntax(filename)
        if not code:
            return
        flist = self.editwin.flist
        shell = flist.open_shell()
        interp = shell.interp
        if PyShell.use_subprocess:
            shell.restart_shell()
        dirname = os.path.dirname(filename)
        # XXX Too often this discards arguments the user just set...
        interp.runcommand("""if 1:
            _filename = %s
            import sys as _sys
            from os.path import basename as _basename
            if (not _sys.argv or
                _basename(_sys.argv[0]) != _basename(_filename)):
                _sys.argv = [_filename]
            import os as _os
            _os.chdir(%s)
            del _filename, _sys, _basename, _os
            \n""" % (`filename`, `dirname`))
        interp.prepend_syspath(filename)
        interp.runcode(code)

    def getfilename(self):
        """Get source filename.  If not saved, offer to save (or create) file

        The debugger requires a source file.  Make sure there is one, and that
        the current version of the source buffer has been saved.  If the user
        declines to save or cancels the Save As dialog, return None.

        If the user has configured IDLE for Autosave, the file will be
        silently saved if it already exists and is dirty.

        """
        filename = self.editwin.io.filename
        if not self.editwin.get_saved():
            autosave = idleConf.GetOption('main', 'General',
                                          'autosave', type='bool')
            if autosave and filename:
                self.editwin.io.save(None)
            else:
                reply = self.ask_save_dialog()
                self.editwin.text.focus_set()
                if reply == "ok":
                    self.editwin.io.save(None)
                    filename = self.editwin.io.filename
                else:
                    filename = None
        return filename

    def ask_save_dialog(self):
        msg = "Source Must Be Saved\n" + 5*' ' + "OK to Save?"
        mb = tkMessageBox.Message(title="Save Before Run or Check",
                                  message=msg,
                                  icon=tkMessageBox.QUESTION,
                                  type=tkMessageBox.OKCANCEL,
                                  default=tkMessageBox.OK,
                                  master=self.editwin.text)
        return mb.show()

    def errorbox(self, title, message):
        # XXX This should really be a function of EditorWindow...
        tkMessageBox.showerror(title, message, master=self.editwin.text)
        self.editwin.text.focus_set()