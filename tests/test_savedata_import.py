"""Importing saves from another installation: every accepted source folder, the backup of the
saves already present, and the cases with nothing to import."""
from paths import ROOT
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import savedata_import


def make_saves(root: Path, user='1', title='CUSA00207', text='old') -> Path:
    """An installation's user folder with one save, as the port writes it."""
    slot = root / 'user' / 'savedata' / user / title / 'SPRJ0005'
    slot.mkdir(parents=True)
    (slot / 'userdata0000').write_text(text)
    meta = root / 'user' / 'savedata' / user / title / 'SPRJ0005.sce_sys'
    meta.mkdir()
    (meta / 'param.bin').write_text('param')
    memory = root / 'user' / 'savedata' / user / f'{title}.memory'
    memory.mkdir()
    (memory / 'memory.dat').write_text('memory')
    return root / 'user'


class SavedataImport(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.old_user = make_saves(self.root / 'old')
        self.new_user = self.root / 'new' / 'user'

    def tearDown(self):
        self.temp.cleanup()

    def slot(self, user_dir, user='1'):
        return user_dir / 'savedata' / user / 'CUSA00207' / 'SPRJ0005' / 'userdata0000'

    def test_every_source_folder_is_accepted(self):
        savedata = self.old_user / 'savedata'
        for source in (self.root / 'old', self.old_user, savedata, savedata / '1', savedata / '1' / 'CUSA00207'):
            found = savedata_import.find_saves(source)
            self.assertEqual(list(found), ['1'], source)
            self.assertEqual(found['1'].resolve(), (savedata / '1').resolve(), source)

    def test_import_into_an_empty_installation(self):
        summary = savedata_import.import_saves(self.root / 'old', self.new_user)
        self.assertEqual(self.slot(self.new_user).read_text(), 'old')
        self.assertTrue((self.new_user / 'savedata/1/CUSA00207/SPRJ0005.sce_sys/param.bin').exists())
        self.assertTrue((self.new_user / 'savedata/1/CUSA00207.memory/memory.dat').exists())
        self.assertNotIn('backup', summary)
        # The source is copied, not moved.
        self.assertEqual(self.slot(self.old_user).read_text(), 'old')

    def test_existing_saves_are_backed_up_not_overwritten(self):
        make_saves(self.root / 'new', text='current')
        savedata_import.import_saves(self.old_user, self.new_user)
        self.assertEqual(self.slot(self.new_user).read_text(), 'old')
        backups = list(self.new_user.glob('savedata_backup_*'))
        self.assertEqual(len(backups), 1)
        self.assertEqual((backups[0] / '1/CUSA00207/SPRJ0005/userdata0000').read_text(), 'current')

    def test_other_users_saves_are_left_alone(self):
        make_saves(self.root / 'new', user='2', text='second user')
        savedata_import.import_saves(self.old_user, self.new_user)
        self.assertEqual(self.slot(self.new_user, '2').read_text(), 'second user')
        self.assertEqual(list(self.new_user.glob('savedata_backup_*')), [])

    def test_nothing_to_import(self):
        empty = self.root / 'empty'
        (empty / 'user' / 'savedata').mkdir(parents=True)
        self.assertEqual(savedata_import.find_saves(empty), {})
        self.assertEqual(savedata_import.find_saves(self.root / 'missing'), {})
        with self.assertRaises(FileNotFoundError):
            savedata_import.import_saves(empty, self.new_user)

    def test_importing_an_installation_into_itself_is_refused(self):
        with self.assertRaises(ValueError):
            savedata_import.import_saves(self.old_user, self.old_user)
        self.assertEqual(self.slot(self.old_user).read_text(), 'old')
        self.assertEqual(list(self.old_user.glob('savedata_backup_*')), [])

    def test_command_line(self):
        script = str(ROOT / 'scripts' / 'savedata_import.py')
        check = subprocess.run([sys.executable, script, str(self.root / 'old'), '--user', str(self.new_user), '--check'],
                               capture_output=True, text=True)
        self.assertEqual(check.returncode, 0, check.stdout + check.stderr)
        self.assertFalse(self.new_user.exists())
        done = subprocess.run([sys.executable, script, str(self.root / 'old'), '--user', str(self.new_user)],
                              capture_output=True, text=True)
        self.assertEqual(done.returncode, 0, done.stdout + done.stderr)
        self.assertEqual(self.slot(self.new_user).read_text(), 'old')
        none = subprocess.run([sys.executable, script, str(self.root / 'nothing'), '--user', str(self.new_user)],
                              capture_output=True, text=True)
        self.assertEqual(none.returncode, 1)


if __name__ == '__main__':
    unittest.main()
