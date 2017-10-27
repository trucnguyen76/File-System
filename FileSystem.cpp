//
// Created by Elva on 10/8/2017.
//

#include "FileSystem.h"

FileSystem::FileSystem() {}
FileSystem::~FileSystem(){}

void FileSystem::create_MASKMAP(){
    //initialize MASK
    MASK[31] = 1;
    for(int i = 30; i >= 0; i--) {
        MASK[i] = MASK[i + 1] << 1 ;
    }
}

void FileSystem::initialize_OFT(){
    //Initialize OFT
    for(int i = 0; i < 4; i++){
        OFT[i].index = -1;
    }
}

void FileSystem::initialize_Bitmap()
{
    char ptr[64];
    ioSystem.read_block(0, ptr);
    int *bitMap = ((int*) ptr);

    create_MASKMAP();

    //If bit is < 32 -> still in the first int which is bitMap[0]
    bitMap[0] = 0;
    bitMap[1] = 0;
    bitMap[0] = bitMap[0] | MASK[0];
    bitMap[0] = bitMap[0] | MASK[1];
    bitMap[0] = bitMap[0] | MASK[2];
    bitMap[0] = bitMap[0] | MASK[3];
    bitMap[0] = bitMap[0] | MASK[4];
    bitMap[0] = bitMap[0] | MASK[5];
    bitMap[0] = bitMap[0] | MASK[6];

    ioSystem.write_block(0, ptr);
}

//This function initialize directory fd as a regular fd which mean it is still free
void FileSystem::initialize_fileDescriptors() {
    char char_ptr[64];
    fileDescriptor* fd_ptr = (fileDescriptor*)(char_ptr);

    //There are a total of 24 files max - > 6 blocks
    for(int block_index = 1; block_index < 7; block_index++) {
        //ioSystem.read_block(block_index, char_ptr);
        //There are 4 fds each block
        for(int j = 0; j < 4; j++){
            fd_ptr[j].length = -1;
            fd_ptr[j].index1 = -1;
            fd_ptr[j].index2 = -1 ;
            fd_ptr[j].index3 = -1;
        }
        ioSystem.write_block(block_index, char_ptr);
    }
}

void FileSystem::initialize_directory() {
    char array[64];

    OFT[0].current_pos = 0;
    OFT[0].index = 0;
    OFT[0].length = 0;

    //Update dir file descriptor
    ioSystem.read_block(1, array);
    ((fileDescriptor*)(array))[0].length = 0;
    ((fileDescriptor*)(array))[0].index1 = 7;
    ioSystem.write_block(1, array);

    //Update bit map since block 7 is taken
    ioSystem.read_block(0, array);
    int *bitMap = ((int*) array);

    //If bit is < 32 -> still in the first int which is bitMap[0]
    bitMap[0] = bitMap[0] | MASK[7];
    ioSystem.write_block(0, array);
}

int FileSystem::allocate_ldisk_block(){
    bool find = false;
    int bit = 0;
    char block[64];
    ioSystem.read_block(0, block);
    int* bitMap = (int*) block;

    while(!find && bit < 64)
    {
        find = ((bitMap[bit / 32] & MASK[bit%32]) == 0);
        if(find) {
            bitMap[bit / 32] = bitMap[bit/32] | MASK[bit%32];
            ioSystem.write_block(0, block);
            //cout << "Allocate free bit: " << bit << endl;
            return bit;
        }
        bit++;
    }
    ioSystem.write_block(0, block);
    return -1;
}

void FileSystem::free_ldisk_block(int bit){
    char block[64];
    int *bitMap = (int*) block;

    ioSystem.read_block(0, block);
    bitMap[bit/32] = bitMap[bit/32]  & (~MASK[bit % 32]);
    ioSystem.write_block(0, block);
}

int FileSystem::create(char symbolic_file_name[]){   //IN - symbolic file name
    //This will reuse fd if they are used and set back to free
    bool free_fd_found = false;
    int block_index = 1;
    int bytes_index_in_block = 0;
    char array[64];
    int fd;

    if(symbolic_file_name[4] != 0 && symbolic_file_name[3] != 0 && symbolic_file_name[2] != 0 && symbolic_file_name[1] != 0){
        cout << "CREATE ERROR!!!Exceed maximum length for symbolic file name!\n";
        return -1;
    }

    while(!free_fd_found && block_index < 7) {
        ioSystem.read_block(block_index, array);

        bytes_index_in_block = 0;
        while(!free_fd_found && bytes_index_in_block < 64)
        {
            //Check length field in fda
            if(((fileDescriptor*)(&(array[bytes_index_in_block])))->length == -1)
                 free_fd_found = true;
            else
                bytes_index_in_block += 16;
        }
        if(!free_fd_found)
            block_index++;
    }

    if(free_fd_found) {
        fd = (block_index - 1) * 4 + bytes_index_in_block / 16;
    }
    else {
        cout << "Error!!! File System is full\n" << endl;
        return -1;
    }

//Find free directory entry
    char dir_entry_ptr[8];
    bool isFree = false;
    bool sameName = true;

    //read return 0 when it hits EOF
    lseek(0,0);
    while(!isFree && read(0, dir_entry_ptr, 8) != 0){
        isFree = ((dir_entry*)(dir_entry_ptr))->fd == -1;
        //Check for same name
        if(((dir_entry*)(dir_entry_ptr))->fd != -1){
            //cout << "In checking same name\n";
            int i = 0;
            sameName = true;
            while(i < 4 && sameName){
                sameName = ((dir_entry*)(dir_entry_ptr))->symbolic_file_name[i] ==  symbolic_file_name[i];
                i++;
            }
            if(sameName) {
                cout << "CREATE ERROR!!!File already exists!\n";
                return -1;
            }
        }
    }
    dir_entry new_entry;
    if(isFree){
        lseek(0, (fd - 1) * 8);
    }
    //If there is none that is free but the size of directory is not full yet
    if(isFree || OFT[0].length != MAX_FILE_LENGTH){
        //Fill in entries
        new_entry;
        for(int  i = 0; i < 4; i++)
            new_entry.symbolic_file_name[i] = symbolic_file_name[i];
        new_entry.fd = fd;

        write(0,(char*)(&new_entry), sizeof(dir_entry));

        //Reserve the file descriptor
        ioSystem.read_block(block_index, array);
        ((fileDescriptor*)(array))[fd % 4].length = 0;
        ioSystem.write_block(block_index, array);
    }
    /*if(!isFree && OFT[0].length != MAX_FILE_LENGTH){
        OFT[0].length += 8;
        ioSystem.read_block(1, array);
        ((fileDescriptor*)(array))[0].length = OFT[0].length;
        ioSystem.write_block(1, array);
    }*/
    return 0;
}
int FileSystem::destroy(char symbolic_file_name[]) {  //IN - symbolic file name
    const int INVALID = -1;

    int temp = 0;
    char dir_entry_ptr[8];
    int fd_index = -1;
    bool sameName;
    int index;

    lseek(0, 0);
    while (fd_index == -1 && read(0, dir_entry_ptr, 8) != 0) {
        sameName = true;
        index = 0;
        while(sameName && index < 4){
            sameName = (((dir_entry *)(dir_entry_ptr))->symbolic_file_name[index] == symbolic_file_name[index]);
            index++;
        }
        if (sameName) {
            fd_index = ((dir_entry *) (dir_entry_ptr))->fd;
        }
    }
    if (fd_index == -1) {
        cout << "DESTROY ERROR!!!FILE IS NOT FOUND\n";
        return -1;
    }

    while (temp < 4 && OFT[temp].index != fd_index)
        temp++;

    if (temp != 4)  close(temp);

    //remove directory entry
    lseek(0, OFT[0].current_pos - 4);
    write(0, (char*)&INVALID, 4);
    fileDescriptor block[4];

    int block_list[3];
    //Update bit map (if file was not empty)
    int fd_index_in_block = fd_index % 4;
    int block_index_fd_is_in = fd_index / 4 + 1;

    ioSystem.read_block(block_index_fd_is_in, (char*)block);

    block_list[0] = block[fd_index_in_block].index1;
    block_list[1] = block[fd_index_in_block].index2;
    block_list[2] = block[fd_index_in_block].index3;

    for(int i = 0; i < 3; i++){
        if(block_list[i] != -1)
            free_ldisk_block(block_list[i]);
    }
    //Free file descriptor
    block[fd_index_in_block].length = -1;
    block[fd_index_in_block].index1 = -1;
    block[fd_index_in_block].index2 = -1;
    block[fd_index_in_block].index3 = -1;
    ioSystem.write_block(block_index_fd_is_in, (char*) block);
    return 0;
}

int FileSystem::open(char symbolic_file_name[]){      //IN - symbolic file name
    int OFT_index;
    int fd_index = -1;
    int block_index; //the index that the block which has the wanted fd in it
    char block[64];
    char dir_entry_ptr[8];
    char name[5];

    //Search/Read directory to find index of file descriptor
    //read return 0 when it hits EOF
    lseek(0,0);
    while(read(0, dir_entry_ptr, 8) != 0){
        for(int i = 0; i < 4; i++){
            name[i] = ((dir_entry*)(dir_entry_ptr))->symbolic_file_name[i];
        }
        name[4] = 0;
        if(strcmp(name , symbolic_file_name) == 0 && ((dir_entry*)(dir_entry_ptr))->fd != -1){
            fd_index = ((dir_entry*)(dir_entry_ptr))->fd;
        }
    }
    if(fd_index == -1) {
        cout << "Open ERROR!!!File does not exist!!!\n";
        return -1;
    }

    //Check if the file is already opened
    for(int i = 0; i < 4; i++){
        if(OFT[i].index == fd_index){
            cout << "OPEN ERROR!!! FILE IS ALREADY OPEN\n";
            return -1;
        }
    }

    //Allocate free OFT entry
    OFT_index = 1;
    while (OFT_index < 4 && OFT[OFT_index].index != -1){
        OFT_index++;
    }

    if(OFT_index == 4) {
        cout << "Can't open file. OFT is full atm!!!\n";
        return -1;
    }
    else{
        fileDescriptor* fd_ptr;
        //Fill in current position and file descriptor index
        OFT[OFT_index].index = fd_index;
        OFT[OFT_index].current_pos = 0;

        //Check if there is memory allocate for it or not.
        /*******************************************************************
         * Read block 0 of file into r/w buffer:
         *    Find the index that the block that has the wanted fd in it
         *    Have a fileDescriptor pointer point to that ldisk byte
         *    Read the first block into the buffer
         *    Read the length of the file into the OFT_entry
         ********************************************************************/
        block_index = (fd_index / 4) + 1;
        ioSystem.read_block(block_index, block);
        fd_ptr = &(((fileDescriptor*)(block))[fd_index % 4]);
        int newBlock;
        if(fd_ptr->index1 == -1) {
            newBlock = allocate_ldisk_block();
            fd_ptr->index1 = newBlock;
        }

        //Update index1 of the file
        ioSystem.write_block(block_index, block);
        ioSystem.read_block(fd_ptr->index1, OFT[OFT_index].buffer);
        OFT[OFT_index].length = fd_ptr->length;
        /*
        for(int i = 0; i < 4; i++){
            cout << OFT[i].index << endl;
        }*/
        return OFT_index;
    }
}
int FileSystem::close(int OFT_index) {          //IN - OFT index
    int current_pos_block;
    if (OFT_index >= 0 && OFT_index < 4) {
        //Write buffer to disk
        fileDescriptor block[4];
        if(OFT[OFT_index].index == -1){
            cout << "CLOSE ERROR!!! FILE ALREADY CLOSED!\n";
            return -1;
        }

        ioSystem.read_block(OFT[OFT_index].index / 4 + 1, (char*)block);

        //Since the current_pos is the one will be read, its not checked if its out of bound yet
        //-> have to check if its >= 64, if it is then we have to change the block
        if(OFT[OFT_index].current_pos % 64 != 0 || OFT[OFT_index].current_pos == 0)
            current_pos_block = OFT[OFT_index].current_pos / 64;
        else
            current_pos_block = (OFT[OFT_index].current_pos - 1) / 64;

        switch(current_pos_block){
            case 0:
                ioSystem.write_block(block[OFT[OFT_index].index % 4].index1, OFT[OFT_index].buffer);
                break;
            case 1:
                ioSystem.write_block(block[OFT[OFT_index].index % 4].index2, OFT[OFT_index].buffer);
                break;
            case 2:
                ioSystem.write_block(block[OFT[OFT_index].index % 4].index3, OFT[OFT_index].buffer);
                break;
        };

        //Update file length in descriptor
        block[OFT[OFT_index].index % 4].length = OFT[OFT_index].length;

        //Free OFT entry
        OFT[OFT_index].index = -1;
/*
        for(int i = 0; i < 4; i++)
            cout << OFT[i].index << endl;
*/
        return 0;
    }
    else
        return -1;
}
//RETURN: # bytes read
int FileSystem::read(int OFT_index,         //IN - OFT index
                     char* mem_area,        //IN - the memory block which it copy to
                     int count)             //IN - number of bytes to read
{
    int inplace_index = OFT[OFT_index].current_pos % 64;
    int data_block_number;  //The block's number of a file that is currently in the buffer (1, 2 or 3)
    int current_block;      //The index of the ldisk block that that is currently in buffer (7-63)
    int next_block;         //The index of the ldisk block that would be next in the buffer (7-63)
    int bytesRead;

    if(OFT[OFT_index].index == -1){
        cout << "Error! There is no open file at index " << OFT_index << endl;
        return -1;
    }

    for(bytesRead = 0;
        bytesRead < count && (OFT[OFT_index].current_pos + bytesRead) < OFT[OFT_index].length;
        bytesRead++){

        //If the current read is still inside the buffer
  //      if((OFT[OFT_index].current_pos + bytesRead)% 64 != 0 || (OFT[OFT_index].current_pos + bytesRead) == 0) {
  //          mem_area[bytesRead] = OFT[OFT_index].buffer[inplace_index + bytesRead];
    //    }else
        if((OFT[OFT_index].current_pos + bytesRead)% 64 == 0 && (OFT[OFT_index].current_pos + bytesRead) != 0) {
            data_block_number = (OFT[OFT_index].current_pos + bytesRead - 1) / 64;
            fileDescriptor block[4];
            ioSystem.read_block(OFT[OFT_index].index / 4 + 1, (char*)block);
            switch(data_block_number){
                case 0:
                    current_block = block[OFT[OFT_index].index % 4].index1;
                    next_block = block[OFT[OFT_index].index % 4].index2;
                    break;
                case 1:
                    current_block = block[OFT[OFT_index].index % 4].index2;
                    next_block = block[OFT[OFT_index].index % 4].index3;
                    break;
                case 2:
                    current_block = block[OFT[OFT_index].index % 4].index3;
                    next_block = -1;
                    break;
                default:
                    cerr << "ERROR!!Current block is more than 2\n";
                    break;
            };

            ioSystem.write_block(current_block, OFT[OFT_index].buffer);
            ioSystem.read_block(next_block, OFT[OFT_index].buffer);
            //Since it move on to a new block, inplace_index will have to be -bytesRead
            // to cancel out +bytesRead and start back at the beginning of buffer
            inplace_index = -bytesRead;
        }
        mem_area[bytesRead] = OFT[OFT_index].buffer[inplace_index + bytesRead];
    }
    //current position will be at the one not read yet
    OFT[OFT_index].current_pos += bytesRead;
    return bytesRead;
}

//RETURN: # bytes written
int FileSystem::write(  int OFT_index,              //IN - OFT index
                        char* mem_area,             //IN -
                        int count)                  //IN -
{
    //Compute position in r/w buffer
    int inplace_index = OFT[OFT_index].current_pos % 64;
    int data_block_number;  //The block's number of a file that is currently in the buffer (1, 2 or 3)
    int current_block;      //The index of the ldisk block that that is currently in buffer (7-63)
    int next_block;         //The index of the ldisk block that would be next in the buffer (7-63)
    int bytesWrite;
    int mem_area_index = 0;
    fileDescriptor block[4];

    if(OFT[OFT_index].index == -1){
        cout << "Error! There is no open file at index " << OFT_index << endl;
        return -1;
    }
    for(bytesWrite = 0;
        bytesWrite < count && (OFT[OFT_index].current_pos + bytesWrite) < MAX_FILE_LENGTH;
        bytesWrite++){

        if(OFT_index != 0 && mem_area[mem_area_index] == 0)
            mem_area_index = 0;

        //If the current read is still inside the buffer
        if((OFT[OFT_index].current_pos + bytesWrite)% 64 != 0 || (OFT[OFT_index].current_pos + bytesWrite) == 0) {
            OFT[OFT_index].buffer[inplace_index + bytesWrite] = mem_area[mem_area_index];
        }
        //It will only in else if index is 64n
        else{
            next_block = -1;
            data_block_number = (OFT[OFT_index].current_pos + bytesWrite - 1) / 64;
            ioSystem.read_block((OFT[OFT_index].index / 4) + 1, (char*)block);

            switch(data_block_number){
                case 0:
                    current_block = block[OFT[OFT_index].index % 4].index1;

                    if(block[OFT[OFT_index].index % 4].index2 == -1){
                        //allocate a new block
                        block[OFT[OFT_index].index % 4].index2 = allocate_ldisk_block();
                    }
                    next_block = block[OFT[OFT_index].index % 4].index2;
                    break;
                case 1:
                    current_block = block[OFT[OFT_index].index % 4].index2;
                    if(block[OFT[OFT_index].index % 4].index3 == -1){
                        //allocate a new block
                        block[OFT[OFT_index].index % 4].index3 = allocate_ldisk_block();
                    }
                    next_block = block[OFT[OFT_index].index % 4].index3;
                    break;
                case 2:
                    current_block = block[OFT[OFT_index].index % 4].index3;
                    next_block = -1;
                    break;
                default:
                    cout << "ERROR!!Current block is more than 2\n";
                    break;
            };
            ioSystem.write_block((OFT[OFT_index].index / 4) + 1, (char*)block);
            ioSystem.write_block(current_block, OFT[OFT_index].buffer);

            if(next_block == -1) {
                cout << "WRITE ERROR!!!! Out of ldisk block\n";
                cout << "WRITE ERROR " << OFT[OFT_index].current_pos + bytesWrite << endl;
            }
            else
                ioSystem.read_block(next_block, OFT[OFT_index].buffer);

            //Since it move on to a new block, inplace_index will have to be -bytesWrite
            // to cancel out +bytesWrite and start back at the beginning of buffer
            inplace_index = -bytesWrite;
            OFT[OFT_index].buffer[inplace_index + bytesWrite] = mem_area[mem_area_index];
        }
        mem_area_index++;
    }
    //Update data in buffer to current block
    data_block_number = (OFT[OFT_index].current_pos + bytesWrite - 1) / 64;
    ioSystem.read_block((OFT[OFT_index].index / 4) + 1, (char*)block);

    switch(data_block_number){
        case 0:
            current_block = block[OFT[OFT_index].index % 4].index1;
            break;
        case 1:
            current_block = block[OFT[OFT_index].index % 4].index2;
            break;
        case 2:
            current_block = block[OFT[OFT_index].index % 4].index3;
            break;
        default:
            cout << "ERROR!!Current block is more than 2\n";
            break;
    };
    ioSystem.write_block(current_block, OFT[OFT_index].buffer);

    //current position will be at the one not read yet
    OFT[OFT_index].current_pos += bytesWrite;
    //If current_pos < length -> overwrite data -> no need to update length
    if(OFT[OFT_index].current_pos >= OFT[OFT_index].length) {
        OFT[OFT_index].length = OFT[OFT_index].current_pos;
    }
    //write back the length
    //Update the new data if there a new block allocated
    ioSystem.read_block(OFT[OFT_index].index / 4 + 1, (char*)block);
    block[OFT[OFT_index].index % 4].length = OFT[OFT_index].length;
    ioSystem.write_block((OFT[OFT_index].index / 4) + 1, (char*)block);

    return bytesWrite;
}

int FileSystem::lseek(int OFT_index,
                       int pos){
    int current_pos_block;
    int new_pos_block = pos / 64;
    int data_block_number;              //The block's number of a file that is currently in the buffer (1, 2 or 3)
    int current_block;                  //The index of the ldisk block that that is currently in buffer (7-63)
    int new_block;                      //The index of the ldisk block that contains the new position (7-63)

    //Since the current_pos is the one will be read, its not checked if its out of bound yet
    //-> have to check if its >= 64, if it is then we have to change the block
    if(OFT_index > 4 || OFT_index < 0) {
        cout << "Seek Error! OFT index out of bound\n";
        return -1;
    }else if(OFT[OFT_index].index == -1){
        cout << "Seek Error! File is not opened\n";
        return -1;
    }else if(pos < 0 || pos >= OFT[OFT_index].length){
        cout << "Seek Error! Position out of bound\n";
        return -1;
    }

    if(OFT[OFT_index].current_pos % 64 != 0 || OFT[OFT_index].current_pos == 0)
        current_pos_block = OFT[OFT_index].current_pos / 64;
    else
        current_pos_block = (OFT[OFT_index].current_pos - 1) / 64;

    if(pos < OFT[OFT_index].length){
        //If the new position is not within the current block
        if(current_pos_block != new_pos_block){
            //Write buffer to block
            fileDescriptor block[4];
            ioSystem.read_block(OFT[OFT_index].index / 4 + 1, (char*)block);
            switch(current_pos_block){
                case 0:
                    current_block = block[OFT[OFT_index].index % 4].index1;
                    break;
                case 1:
                    current_block = block[OFT[OFT_index].index % 4].index2;
                    break;
                case 2:
                    current_block = block[OFT[OFT_index].index % 4].index3;
                    break;
                default:
                    cerr << "ERROR!!Current block is more than 2\n";
                    break;
            };
            ioSystem.write_block(current_block, OFT[OFT_index].buffer);

            //Read the new block
            data_block_number = pos / 64;
            switch(data_block_number){
                case 0:
                    new_block = block[OFT[OFT_index].index % 4].index1;
                    break;
                case 1:
                    new_block = block[OFT[OFT_index].index % 4].index2;
                    break;
                case 2:
                    new_block = block[OFT[OFT_index].index % 4].index3;
                    break;
                default:
                    cerr << "ERROR!!Current block is more than 2\n";
                    break;
            };
            ioSystem.read_block(new_block,OFT[OFT_index].buffer);
        }
        OFT[OFT_index].current_pos = pos;
        return OFT[OFT_index].current_pos;
    }else if(pos == 0){
        OFT[OFT_index].current_pos = 0;
        return 0;
    }
    else{
        cout << "LSEEK ERROR! New position is outside of file!!\n";
        return -1;
    }
}
//RETURN: list of files
string FileSystem::directory() {
    dir_entry block[24];
    string list;
    lseek(0,0);
    read(0, (char*)block, OFT[0].length);
    for(int i  = 0; i < OFT[0].length / 8; i++){
        if(block[i].fd != -1) {
            for (int j = 0; j < 4 && block[i].symbolic_file_name[j] != 0; j++)
                list.append(1,block[i].symbolic_file_name[j]);
            list.append(1, '\n');
        }
    }
    return list;
}
//This function will restore ldisk from a file or create new if no file is given
void FileSystem::init(char* file_name)     //IN - file to restore the ldisk from or create new if there is no file
{
    initialize_OFT();
    create_MASKMAP();

    if(file_name == nullptr){
        initialize_Bitmap();
        initialize_fileDescriptors();
        initialize_OFT();
        initialize_directory();
    } else{
        ifstream iFile(file_name, ifstream::in);
        char buffer[64];
        int block_number;
        fileDescriptor *fd;
        if(iFile.fail()) {
            cout << ("Error opening file! File does not exist!");
            initialize_Bitmap();
            initialize_fileDescriptors();
            initialize_OFT();
            initialize_directory();
        } else{
            block_number = 0;
            while(!iFile.eof() && block_number < 64){
                cout << "Block number: " << block_number << endl;
                for(int i = 0; i < 64; i++){
                    buffer[i] = iFile.get();
                }
                ioSystem.write_block(block_number++, buffer);
            }
            //Block 7 is the first block of directory always
            ioSystem.read_block(7, buffer);
            for(int i = 0; i < 64; i++){
                OFT[0].buffer[i] = buffer[i];
            }
            OFT[0].index = 0;
            OFT[0].current_pos = 0;
            ioSystem.read_block(1, buffer);
            OFT[0].length = ((fileDescriptor*) (buffer))[0].length;
        }
        iFile.close();
    }
}

//This function will save ldisk to a file
void FileSystem::save(char* file_name)     //IN - file to save ldisk to
{
    ofstream oFile(file_name, ofstream::out);
    char buffer[64];
    int index;
    if(oFile.fail())
        perror("Error opening file!");
    else{
        for(int i = 0; i < 4; i++){
            if(OFT[i].index != -1) close(i);
        }
        for (index = 0; index < 64; index++){
            ioSystem.read_block(index, buffer);
            oFile.write(buffer, 64);
        }
        oFile.close();
    }
}

void FileSystem::printFd(){
    fileDescriptor block[4];
    for(int i = 1; i < 7; i++){
        ioSystem.read_block(i, (char*)block);
        for(int j = 0; j < 4; j++){
            cout << "Index: " << (i-1)*4 + j << endl;
            cout << "Length: " << block[j].length << endl;
            cout << "Index1: " << block[j].index1 << endl;
            cout << "Index2: " << block[j].index2 << endl;
            cout << "Index3: " << block[j].index3 << endl;
        }
    }
}