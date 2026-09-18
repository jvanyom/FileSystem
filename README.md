# FileSystem

A file system written in C and inspired by ext2 concepts. It uses a regular file as a storage device, keeping its own metadata, directories, and file contents inside it. The command-line tools let you format this virtual disk and manage the files it contains.

The project shows how operations on file paths connect to inode and block management. It uses its own storage format: the generated images are not compatible with standard ext2 tools and cannot be mounted as a kernel file system.

## Features

- Create virtual disks, files, and directories.
- Read and write data at a given byte offset, allocating blocks as needed.
- Inspect metadata, including type, permissions, logical size, allocated blocks, link count, and access and modification times.
- Change permissions, create hard links to files, and remove directory entries.
- Rename, move, and copy files and directories, and remove directories recursively.
- Truncate files through the inode API and the corresponding test utility.
- Cache path-to-inode lookups using FIFO or LRU. The current configuration uses LRU with up to three entries per process.

## Code structure

| Files | Purpose |
| --- | --- |
| `blocks.c`, `blocks.h` | Open and close the device, and read and write physical blocks. The current configuration uses `mmap`; an alternative implementation uses `lseek`, `read`, and `write`. |
| `basic_files.c`, `basic_files.h` | Manage the superblock and bitmap, allocate and release blocks and inodes, and map logical blocks to physical blocks. |
| `files.c`, `files.h` | Provide inode operations for reading, writing, metadata access, permission changes, and truncation. |
| `directories.c`, `directories.h` | Resolve paths, manage directory entries and links, and perform file and directory operations. |
| `dcache.c`, `dcache.h` | Maintain an in-memory cache for path lookups. |
| `errors.c`, `errors.h` | Define error codes and diagnostic messages. |
| `my_*.c` | Provide command-line programs for file system operations. |
| `test/` | Contains the Makefile, inspection utilities, and scripts for test scenarios. |

### Virtual disk layout

The file that represents the disk is divided into **1024-byte blocks**:

```text
Superblock | Bitmap | Inode table | Data blocks
```

The superblock stores the location of each region, free space counters, and the root inode. The bitmap tracks which blocks are in use. Inodes have a defined size of **128 bytes** and contain metadata, **12 direct pointers**, and three indirect pointers for single, double, and triple indirection.

Directories store entries that associate a name with an inode number. Formatting reserves one inode for every four disk blocks and creates the root directory `/`.

## Building

You need a Linux/POSIX environment with **GCC**, **GNU Make**, and **Bash**. The Makefile uses C with GNU extensions (`-std=gnu99`). On Windows, you can use a Linux distribution through WSL with these tools installed.

From the repository root, run:

```bash
make -C test
```

The `my_*` executables are created in the repository root, while the supporting utilities are created in `test/`.

The repository also includes `bash build.sh`, which cleans and rebuilds the project while hiding standard output. Use the command above to see the build messages directly. The `clean` target also removes disk images matching `disco*` in the root and `test/` directories, and files matching `ext*` inside `test/`.

## Usage example

Run the following commands from the repository root. `demo_disk` is the file on the host system that holds the virtual disk; `/documents/note.txt` is a path inside that disk.

```bash
# Create and format a disk with 100000 blocks (102400000 bytes).
./my_mkfs demo_disk 100000

# Create a directory and a file with read and write permissions.
./my_mkdir demo_disk 6 /documents/
./my_touch demo_disk 6 /documents/note.txt

# Write from byte 0 and read the contents back.
./my_write demo_disk /documents/note.txt "Hello, FileSystem" 0
./my_cat demo_disk /documents/note.txt

# List the directory and inspect the file metadata.
./my_ls -l demo_disk /documents/
./my_stat demo_disk /documents/note.txt

# Create a second name for the same inode.
./my_link demo_disk /documents/note.txt /documents/link.txt

# Remove the link, the original file, and the empty directory.
./my_rm demo_disk /documents/link.txt
./my_rm demo_disk /documents/note.txt
./my_rmdir demo_disk /documents/
```

`my_mkfs` overwrites the specified file when formatting, so use a test image. Parent directories must exist before you create entries inside them. Internal paths must be absolute, starting with `/`.

Permissions are represented by a single number from **0 to 7**, combining read (`4`), write (`2`), and execute (`1`). For example, `6` grants read and write access. There are no separate permission sets for owner, group, and others. The execute bit is stored and displayed, but the project does not implement program execution.

## Available commands

In this table, `disk` refers to the file system image. File and directory path arguments refer to locations inside that image.

| Syntax | Operation |
| --- | --- |
| `./my_mkfs disk blocks` | Create and format the virtual disk. |
| `./my_mkdir disk permissions /directory/` | Create a directory. |
| `./my_touch disk permissions /file` | Create a new file. |
| `./my_write disk /file "text" offset` | Write text at the specified byte offset. |
| `./my_cat disk /file` | Display the file contents. |
| `./my_ls [-l] disk /path` | List entries; `-l` includes metadata. |
| `./my_stat disk /path` | Display the inode number and its metadata. |
| `./my_chmod disk permissions /path` | Change permissions. |
| `./my_link disk /file /link` | Create a hard link to an existing file. |
| `./my_rn disk /path new_name` | Rename an entry within its current directory. |
| `./my_mv disk /source /destination/` | Move an entry to an existing directory. |
| `./my_cp disk /source /destination/` | Copy a file or a directory and its contents into an existing directory. |
| `./my_rm disk /file` | Remove a file entry and release its inode when no links remain. |
| `./my_rm -r disk /directory/` | Remove a directory and its contents recursively. |
| `./my_rmdir disk /directory/` | Remove an empty directory. |

The `-l` and `-r` options must appear **before the disk name**. `my_rn` takes a new name, while `my_mv` and `my_cp` take a destination directory.

## Testing and inspection

The `test/` directory includes programs for inspecting the superblock (`leer_sf`), reading and writing by inode, changing permissions, truncating files, and exercising the cache. It also contains scenarios for links, copies, moves, renaming, and recursive removal.

The scripts use relative paths and must be run from `test/`. For example:

```bash
cd test
bash test10.sh
```

`test10.sh` rebuilds the project, creates an image named `disco`, and demonstrates link and entry removal operations, including cases that intentionally produce errors. These scripts support manual inspection: their output needs to be reviewed, and they do not constitute a fully automated test suite. Several scripts run `make clean` and recreate the test images.

## Scope

The storage format writes C structures directly to disk, so it depends on the representation of data types and the architecture in use. It does not include journaling or synchronization for concurrent operations across processes. The project is intended for exploring file system implementation with test images.
