#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <stdio.h>
#include <unistd.h>
#include <ftw.h>

#include "ditool.h"
#include "fsdir.h"
#include "UFS.h"
#include "FileTable.h"

using namespace std;

static const char* get_option(const char** begin, const char** end, const string& option) {
    const char** itr = find(begin, end, option);
    if (itr != end && ++itr != end)
        return *itr;
    return NULL;
}

static bool has_option(const char** begin, const char** end, const string& option) {
    return find(begin, end, option) != end;
}

static void print_help(void) {
    cout << "usage : ditool -im <disk_image_file> [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -h          Print this help." << endl;
    cout << "  -im         Raw disk image file." << endl;
    cout << "  -lsp        List partitions in disk image." << endl;
    cout << "  -p          Partition number to work on." << endl;
    cout << "  -ls         List files in disk image." << endl;
    cout << "  -out <path> Copy files from disk image to <path>." << endl;
    cout << "  -clean      Clean output directory before copying." << endl;
}

static bool ignore(const char* name) {
    return strcmp(".", name) == 0 || strcmp("..", name) == 0;
}

static void touch(FileTable* ft, string path) {
    if(ft) {
        FILE* file = ft->fopen(path, "wb");
        if(file) fclose(file);
    }
}

static string readlink(UFS& ufs, icommon& inode) {
    if(inode.ic_Mun.ic_Msymlink[0])
        return inode.ic_Mun.ic_Msymlink;
    else {
        size_t size = fsv(inode.ic_size);
        char   buffer[size+1];
        ufs.readFile(inode, 0, size, (uint8_t*)buffer);
        buffer[size] = '\0';
        return buffer;
    }
}

static void set_attr(UFS& ufs, map<uint32_t, string>& inode2path, uint32_t ino, string path, FileTable* ft) {
    if(ft) {
        vector<direct> entries = ufs.list(ino);
        
        for(size_t i = 0; i < entries.size(); i++) {
            direct& dirEnt = entries[i];
            icommon inode;
            ufs.readInode(inode, fsv(dirEnt.d_ino));
            
            string dirEntPath = path;
            dirEntPath += "/";
            dirEntPath += dirEnt.d_name;
            
            uint32_t rdev = 0;
            switch(fsv(inode.ic_mode) & IFMT) {
                case IFDIR:       /* directory */
                    if(!(ignore(dirEnt.d_name)))
                        set_attr(ufs, inode2path, fsv(dirEnt.d_ino), dirEntPath, ft);
                    break;
                case IFCHR:       /* character special */
                case IFBLK:       /* block special */
                    rdev = fsv(inode.ic_db[0]);
                    break;

            }
            
            struct stat fstat;
            fstat.st_mode              = fsv(inode.ic_mode);
            fstat.st_uid               = fsv(inode.ic_uid);
            fstat.st_gid               = fsv(inode.ic_gid);
            fstat.st_size              = fsv(inode.ic_size);
            fstat.st_atimespec.tv_sec  = fsv(inode.ic_atime.tv_sec);
            fstat.st_atimespec.tv_nsec = fsv(inode.ic_atime.tv_usec) * 1000;
            fstat.st_mtimespec.tv_sec  = fsv(inode.ic_mtime.tv_sec);
            fstat.st_mtimespec.tv_nsec = fsv(inode.ic_mtime.tv_usec) * 1000;
            fstat.st_rdev              = rdev;
            
            FileAttrs fattr(fstat);
            ft->SetFileAttrs(dirEntPath, fattr);
            
            timeval times[2];
            times[0].tv_sec  = fattr.atime_sec;
            times[0].tv_usec = fattr.atime_usec;
            times[1].tv_sec  = fattr.mtime_sec;
            times[1].tv_usec = fattr.mtime_usec;
            if(ft->chmod(dirEntPath, fstat.st_mode & ~IFMT))
                cout << "Unable to set mode for " << dirEntPath << endl;
            if(ft->utimes(dirEntPath, times))
                cout << "Unable to set times for " << dirEntPath << endl;
        }
    }
}

static void verify_attr(UFS& ufs, map<uint32_t, string>& inode2path, uint32_t ino, string path, FileTable* ft) {
    if(ft) {
        vector<direct> entries = ufs.list(ino);
        
        for(size_t i = 0; i < entries.size(); i++) {
            direct& dirEnt = entries[i];
            icommon inode;
            ufs.readInode(inode, fsv(dirEnt.d_ino));
            
            string dirEntPath = path;
            dirEntPath += "/";
            dirEntPath += dirEnt.d_name;
            
            uint32_t rdev = 0;
            switch(fsv(inode.ic_mode) & IFMT) {
                case IFDIR:       /* directory */
                    if(!(ignore(dirEnt.d_name)))
                        verify_attr(ufs, inode2path, fsv(dirEnt.d_ino), dirEntPath, ft);
                    break;
                case IFCHR:       /* character special */
                case IFBLK:       /* block special */
                    rdev = fsv(inode.ic_db[0]);
                    break;
            }
            
            struct stat fstat;
            ft->Stat(dirEntPath, fstat);
            if(fstat.st_mode != fsv(inode.ic_mode))
                cout << "mode mismatch " << std::oct << fstat.st_mode << " != " << std::oct << fsv(inode.ic_mode) << " " << dirEntPath << endl;
            if(fstat.st_uid != fsv(inode.ic_uid))
                cout << "uid mismatch " << dirEntPath << endl;
            if(fstat.st_gid != fsv(inode.ic_gid))
                cout << "gid mismatch " << dirEntPath << endl;
            if((fsv(inode.ic_mode) & IFMT) != IFDIR) {
                if(fstat.st_size != fsv(inode.ic_size))
                    cout << "size mismatch " << fstat.st_size << " != " << fsv(inode.ic_size) << " " << dirEntPath << endl;
                /*
                if(fstat.st_atimespec.tv_sec != fsv(inode.ic_atime.tv_sec))
                    cout << "atime_sec mismatch " << dirEntPath << " diff:" << (fstat.st_atimespec.tv_sec - fsv(inode.ic_atime.tv_sec)) << endl;
                if(fstat.st_atimespec.tv_nsec != fsv(inode.ic_atime.tv_usec) * 1000)
                    cout << "atime_nsec mismatch " << dirEntPath << " diff:" << (fstat.st_atimespec.tv_nsec - (fsv(inode.ic_atime.tv_usec) * 1000)) << endl;
                 */
                if(fstat.st_mtimespec.tv_sec != fsv(inode.ic_mtime.tv_sec))
                    cout << "mtime_sec mismatch diff:" << (fstat.st_mtimespec.tv_sec << - fsv(inode.ic_mtime.tv_sec)) << " " << dirEntPath << endl;
                if(fstat.st_mtimespec.tv_nsec != fsv(inode.ic_mtime.tv_usec) * 1000)
                    cout << "mtime_nsec mismatch diff:" << (fstat.st_mtimespec.tv_nsec << - (fsv(inode.ic_mtime.tv_usec)) * 1000) << " " << dirEntPath << endl;
            }
            if((uint32_t)fstat.st_rdev != rdev)
                cout << "rdev mismatch " << dirEntPath << endl;
        }
    }
}

static void copy_inode(UFS& ufs, map<uint32_t, string>& inode2path, uint32_t ino, string path, FileTable* ft, ostream& os) {
    vector<direct> entries = ufs.list(ino);
    for(size_t i = 0; i < entries.size(); i++) {
        direct& dirEnt = entries[i];
        icommon inode;
        ufs.readInode(inode, fsv(dirEnt.d_ino));
        
        string dirEntPath = path;
        dirEntPath += "/";
        dirEntPath += dirEnt.d_name;
        bool doPrint = true;
        
        if(!(ignore(dirEnt.d_name))) {
            if(ft->access(dirEntPath, F_OK) == 0)
                cout << "WARNING: file " << dirEntPath << " already exits. Maybe case insensitive host file system or output path not clean" << endl;

            if(inode2path.find(fsv(dirEnt.d_ino)) != inode2path.end()) {
                os << "[HLINK] " << inode2path[fsv(dirEnt.d_ino)] << " - ";
                if(ft) ft->link(inode2path[fsv(dirEnt.d_ino)], dirEntPath, false);
            } else
                inode2path[fsv(dirEnt.d_ino)] = dirEntPath;
        }
        
        switch(fsv(inode.ic_mode) & IFMT) {
            case IFIFO:       /* named pipe (fifo) */
                os << "[FIFO]  ";
                touch(ft, dirEntPath);
                break;
            case IFCHR:        /* character special */
                os << "[CHAR]  ";
                touch(ft, dirEntPath);
                break;
            case IFDIR:       /* directory */
                os << "[DIR]   ";
                if(!(ignore(dirEnt.d_name))) {
                    os << dirEntPath << endl;
                    doPrint = false;
                    if(ft) ft->mkdir(dirEntPath, DEFAULT_PERM);
                    copy_inode(ufs, inode2path, fsv(dirEnt.d_ino), dirEntPath, ft, os);
                }
                break;
            case IFBLK:       /* block special */
                os << "[BLOCK] ";
                touch(ft, dirEntPath);
                break;
            case IFREG:        /* regular */
                os << "[FILE]  ";
                if(ft) {
                    FILE* file = ft->fopen(dirEntPath, "wb");
                    if(file) {
                        size_t size = fsv(inode.ic_size);
                        uint8_t* buffer = new uint8_t[size];
                        ufs.readFile(inode, 0, size, buffer);
                        fwrite(buffer, sizeof(uint8_t), size, file);
                        delete [] buffer;
                        fclose(file);
                    }
                }
                break;
            case IFLNK: {       /* symbolic link */
                string link = readlink(ufs, inode);
                os << "[SLINK] " << link << " - ";
                if(ft) ft->link(link, dirEntPath, true);
                break;
            }
            case IFSOCK:        /* socket */
                os << "[SOCK]  ";
                touch(ft, dirEntPath);
                break;
        }
        

        if(doPrint)
            os << dirEntPath << endl;
    }
}

static void dump_part(DiskImage& im, int part, const char* outPath, ostream& os) {
    FileTable* ft = NULL;
    UFS ufs(im.parts[part]);

    if(outPath)
        ft = new FileTable(outPath, ufs.mountPoint());
    
    if(ft) cout << "---- copying " << im.path << " to " << ft->GetBasePath() << endl;
    map<uint32_t, string> inode2path;
    copy_inode(ufs, inode2path, ROOTINO, "", ft, os);
    set_attr  (ufs, inode2path, ROOTINO, "", ft);
    if(ft) {
        cout << "---- writing file attributes for NFSD" << endl;
        ft->Write();
        cout << "---- verifying file attributes and sizes for NFSD" << endl;
        verify_attr(ufs, inode2path, ROOTINO, "", ft);
        delete ft;
    }
}

static void clean_dir(const char* path) {
    cout << "---- cleaning " << path << endl;
    nftw(path, FileTable::Remove, 1024, FTW_DEPTH | FTW_PHYS);
    if(access(path, F_OK | R_OK | W_OK) == 0) {
        char tmp[32];
        sprintf(tmp, ".%08X", rand());
        string newName = path;
        newName += tmp;
        cout << "directory " << path << " still exists. trying to rename it to " << newName;
        rename(path, newName.c_str());
    }
}

class NullBuffer : public streambuf {
public: int overflow(int c) {return c;};
};

extern "C" int main(int argc, const char * argv[]) {
    if(has_option(argv, argv+argc, "-h") || has_option(argv, argv+argc, "--help"))
        print_help();
    
    const char* imageFile = get_option(argv, argv + argc, "-im");
    bool        listParts = has_option(argv, argv+argc,   "-lsp");
    const char* partNum   = get_option(argv, argv + argc, "-p");
    bool        listFiles = has_option(argv, argv + argc, "-ls");
    const char* outPath   = get_option(argv, argv + argc, "-out");
    bool        clean     = has_option(argv, argv + argc, "-clean");

    if (imageFile) {
        ifstream imf(imageFile, ios::binary | ios::in);
        
        if(!(imf)) {
            cout << "Can't read '" << imageFile << "'." << endl;
            return 1;
        }
        
        DiskImage  im(imageFile, imf);
        NullBuffer nullBuffer;
        ostream    nullStream(&nullBuffer);
        
        if(listParts)
            cout << im << endl;
        
        if(listFiles || outPath) {
            if(outPath) {
                if(clean) clean_dir(outPath);
                mkdir(outPath, DEFAULT_PERM);
                if(access(outPath, F_OK | R_OK | W_OK) < 0) {
                    cout << "Can't access '" << outPath << "'" << endl;
                    return 1;
                }
            }
            
            int part = partNum ? atoi(partNum) : -1;
            if(part >= 0 && part < (int)im.parts.size() && im.parts[part].isUFS()) {
                dump_part(im, part, outPath, listFiles ? cout : nullStream);
            } else {
                for(int part = 0; part < (int)im.parts.size(); part++)
                    dump_part(im, part, outPath, listFiles ? cout : nullStream);
            }
        }
    } else {
        cout << "Missing image file." << endl;
        print_help();
        return 1;
    }
    
    cout << "---- done." << endl;
    return 0;
}