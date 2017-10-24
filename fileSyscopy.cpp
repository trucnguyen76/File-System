////
//// Created by Elva on 10/8/2017.
////
//
//#include "FileSystem.h"
//
//FileSystem::FileSystem() {
//    char* ptr;
//
//    initialize_Bitmap();
//
//    ioSystem.read_block(1, ptr);
//
//    initialize_fileDescriptors();
//
//    //Initialize OFT
//    for(int i = 0; i < 4; i++){
//        OFT[i].index = -1;
//    }
//    initialize_directory();
//}
//FileSystem::~FileSystem(){}
//
//void FileSystem::initialize_Bitmap()
//{
//    char ptr[64];
//    ioSystem.read_block(0, ptr);
//    int *bitMap = ((int*) ptr);
//
//    //ininitialize MASK
//    MASK[31] = 1;
//    for(int i = 30; i >= 0; i--) {
//        MASK[i] = MASK[i + 1] << 1 ;
//    }
//
//    //If bit is < 32 -> still in the first int which is bitMap[0]
//    bitMap[0] = bitMap[0] | MASK[0];
//    bitMap[0] = bitMap[0] | MASK[1];
//    bitMap[0] = bitMap[0] | MASK[2];
//    bitMap[0] = bitMap[0] | MASK[3];
//    bitMap[0] = bitMap[0] | MASK[4];
//    bitMap[0] = bitMap[0] | MASK[5];
//    bitMap[0] = bitMap[0] | MASK[6];
//
//    ioSystem.write_block(0, ptr);
//
//    /*
//    int* ptr = (int*) ldisk[0];
//    //First block dedicated to bitmap -> busy
//    bitMap[0] = bitMap[0] | MASK[0];
//    cout << &(ldisk[0]) << " vs " << &(*bitMap) <<  endl;
//    cout << "Bitmap[0] is " << bitMap[0] << endl;
//    cout << "Ldisk[0] is " << *((int*) (ldisk[0])) << endl;
//     */
//}
//
////This function initialize directory fd as a regular fd which mean it is still free
//void FileSystem::initialize_fileDescriptors() {
//    char char_ptr[64];
//    fileDescriptor* fd_ptr = (fileDescriptor*)(char_ptr);
//
//    //There are a total of 24 files max - > 6 blocks
//    for(int block_index = 1; block_index < 7; block_index++) {
//        //ioSystem.read_block(block_index, char_ptr);
//        //There are 4 fds each block
//        for(int j = 0; j < 4; j++){
//            fd_ptr[j].length = -1;
//            fd_ptr[j].index1 = -1;
//            fd_ptr[j].index2 = -1 ;
//            fd_ptr[j].index3 = -1;
//        }
///*        for(int j = 0; j < 64; j+=16){
//            cout << "Fd[" << j << "] length is: " << *((int*)&(char_ptr[j])) << endl;
//            cout << "Fd[" << j << "] index 1 is: " << *((int*)&(char_ptr[j + 4])) << endl;
//            cout << "Fd[" << j << "] index 2 is: " << *((int*)&(char_ptr[j + 8])) << endl;
//            cout << "Fd[" << j << "] index 3 is: " << *((int*)&(char_ptr[j + 12])) << endl;
//        }
//*/
//        ioSystem.write_block(block_index, char_ptr);
//    }
//    /*
//    char array[64];
//    //Print to check the content fds
//    for(int block_index = 1; block_index < 7; block_index++) {
//        ioSystem.read_block(block_index, array);
//        //ioSystem.read_block(block_index, char_ptr);
//        //There are 4 fds each block
//        for(int j = 0; j < 64; j+=16){
//            cout << "Fd[" << j << "] length is: " << *((int*)&(array[j])) << endl;
//            cout << "Fd[" << j << "] index 1 is: " << *((int*)&(array[j + 4])) << endl;
//            cout << "Fd[" << j << "] index 2 is: " << *((int*)&(array[j + 8])) << endl;
//            cout << "Fd[" << j << "] index 3 is: " << *((int*)&(array[j + 12])) << endl;
//        }
//    }
//*/
//}
//
//void FileSystem::initialize_directory() {
//    char array[64];
//
//    OFT[0].current_pos = 0;
//    OFT[0].index = 0;
//    OFT[0].length = 0;
//
//    //Update dir file descriptor
//    ioSystem.read_block(1, array);
//    ((fileDescriptor*)(array))[0].length = 0;
//    ((fileDescriptor*)(array))[0].index1 = 7;
//    ioSystem.write_block(1, array);
//
//    //Update bit map since block 7 is taken
//    ioSystem.read_block(0, array);
//    int *bitMap = ((int*) array);
//
//    //If bit is < 32 -> still in the first int which is bitMap[0]
//    bitMap[0] = bitMap[0] | MASK[7];
//
//    ioSystem.write_block(0, array);
///*
//    //Test
//    char array1[64];
//    //ioSystem.read_block(0, array1);
//    //cout << "First half of bit map: " << *((int*)(array1)) << endl;
//    ioSystem.read_block(1, array1);
//    cout << "First block of dir: " << ((fileDescriptor*)(array1))->index1 << endl;
//
//*/
//
//}
//
//int FileSystem::allocate_ldisk_block(){
//    bool find = false;
//    int bit = 0;
//    char block[64];
//    ioSystem.read_block(0, block);
//    int* bitMap = (int*) block;
//
//    while(!find && bit < 64)
//    {
//        if(bit < 32) {
//            find = ((bitMap[0] & MASK[bit]) == 0);
//            if(find) {
//                bitMap[0] = bitMap[0] | MASK[bit];
//                ioSystem.write_block(0, block);
//                return bit;
//            }
//        }
//        //else if its in the 2nd half
//        else {
//            find = ((bitMap[1] & MASK[bit - 32]) == 0);
//            if(find) {
//                bitMap[1] = bitMap[1] | MASK[bit - 32];
//                ioSystem.write_block(0, block);
//                return bit;
//            }
//        }
//        bit++;
//    }
//    ioSystem.write_block(0, block);
//    return -1;
//}
//
//void FileSystem::create(char symbolic_file_name[]){   //IN - symbolic file name
//    //This will reuse fd if they are used and set back to free
//    bool free_fd_found = false;
//    int block_index = 1;
//    int index_in_block;
//    char array[64];
//    int fd;
//
//    while(!free_fd_found && block_index < 7)
//    {
//        ioSystem.read_block(block_index, array);
//
//        index_in_block = 0;
//        while(!free_fd_found && index_in_block < 64)
//        {
//            //Check length field in fda
//            if(((fileDescriptor*)(&(array[index_in_block])))->length == -1)
//                 free_fd_found = true;
//            else
//                index_in_block += 16;
//        }
//        if(!free_fd_found)
//            block_index++;
//    }
//
//    if(free_fd_found) {
//        fd = (block_index - 1) * 4 + index_in_block / 16;
//    }
//    else {
//        cout << "Error!!! File System is full\n" << endl;
//        return;
//    }
//
////Find free directory entry
//    char dir_entry_ptr[8];
//    bool isFree = false;
//
//    //read return 0 when it hits EOF
//    lseek(0,0);
//    while(!isFree && read(0, dir_entry_ptr, 8) != 0){
//        isFree = ((dir_entry*)(dir_entry_ptr))->fd == -1;
//        if(strcmp(((dir_entry*)(dir_entry_ptr))->symbolic_file_name, symbolic_file_name) == 0){
//            cout <<"CREATE ERROR!!!File already exists!\n";
//            return;
//        }
//
//    }
//    //If there is none that is free but the sie of directory is not full yet
//    if(!isFree && OFT[0].length != MAX_FILE_LENGTH){
//        //Fill in entries
//        dir_entry new_entry;
//        for(int  i = 0; i < 4; i++)
//            new_entry.symbolic_file_name[i] = symbolic_file_name[i];
//        new_entry.fd = fd;
//        //Write new entry to diectory
//        write(0,(char*)(&new_entry), sizeof(dir_entry));
//
//        //Reserve the file descriptor
//        ((fileDescriptor*)(&array[index_in_block]))->length = 0;
//        ioSystem.write_block(block_index, array);
//    }
//    cout << "**************End of create****************\n";
//}
//void FileSystem::destroy(char symbolic_file_name[]){  //IN - symbolic file name
//}
//
////RETURN: int - OFT index
//int FileSystem::open(char symbolic_file_name[]){      //IN - symbolic file name
//    int OFT_index;
//    int fd_index = -1;
//    int block_index; //the index that the block which has the wanted fd in it
//    char block[64];
//    char dir_entry_ptr[8];
//
//    cout << "******START OPEN************\n";
//    //Search/Read directory to find index of file descriptor
//    //read return 0 when it hits EOF
//    lseek(0,0);
//    while(read(0, dir_entry_ptr, 8) != 0){
//        if(strcmp(((dir_entry*)(dir_entry_ptr))->symbolic_file_name, symbolic_file_name) == 0){
//            fd_index = ((dir_entry*)(dir_entry_ptr))->fd;
//        }
//    }
//    if(fd_index == -1) {
//        cout << "Open ERROR!!!File does not exist!!!\n";
//        return -1;
//    }
//
//    //Check if the file is already opened
//    for(int i = 0; i < 4; i++){
//        if(OFT[i].index == fd_index){
//            cout << "OPEN ERROR!!! FILE IS ALREADY OPEN\n";
//            return -1;
//        }
//
//    }
//
//    //Allocate free OFT entry
//    OFT_index = 1;
//    while (OFT_index < 4 && OFT[OFT_index].index != -1){
//        OFT_index++;
//    }
//
//    if(OFT_index == 4) {
//        cout << "Can't open file. OFT is full atm!!!\n";
//        return -1;
//    }
//    else{
//        fileDescriptor* fd_ptr;
//        //Fill in current position and file descriptor index
//        OFT[OFT_index].index = fd_index;
//        OFT[OFT_index].current_pos = 0;
//
//
//        //Check if there is memory allocate for it or not.
//        /*******************************************************************
//         * Read block 0 of file into r/w buffer:
//         *    Find the index that the block that has the wanted fd in it
//         *    Have a fileDescriptor pointer point to that ldisk byte
//         *    Read the first block into the buffer
//         *    Read the length of the file into the OFT_entry
//         ********************************************************************/
//        block_index = (fd_index / 4) + 1;
//        ioSystem.read_block(block_index, block);
//        //fd_ptr = (fileDescriptor*)(&block[((fd_index - ((block_index - 1) * 4))* 16)]);
//        fd_ptr = &(((fileDescriptor*)(block))[fd_index % 4]);
//        int newBlock;
//        if(fd_ptr->index1 == -1) {
//            newBlock = allocate_ldisk_block();
//            fd_ptr->index1 = newBlock;
//        }
//        //Update index1 of the file
//
//        ioSystem.write_block(block_index, block);
//        ioSystem.read_block(fd_ptr->index1, OFT[OFT_index].buffer);
//        OFT[OFT_index].length = fd_ptr->length;
//        cout << "***********END OPEN*************\n";
//
//        for(int i = 0; i < 4; i++){
//            cout << OFT[i].index << endl;
//        }
//        return OFT_index;
//    }
//}
//int FileSystem::close(int OFT_index) {          //IN - OFT index
//    int current_pos_block;
//    if (OFT_index >= 0 || OFT_index < 4) {
//        //Write buffer to disk
//        fileDescriptor block[4];
//        ioSystem.read_block(OFT[OFT_index].index / 4 + 1, (char*)block);
//
//        //Since the current_pos is the one will be read, its not checked if its out of bound yet
//        //-> have to check if its >= 64, if it is then we have to change the block
//        if(OFT[OFT_index].current_pos % 64 != 0 || OFT[OFT_index].current_pos == 0)
//            current_pos_block = OFT[OFT_index].current_pos / 64;
//        else
//            current_pos_block = (OFT[OFT_index].current_pos - 1) / 64;
//
//        switch(current_pos_block){
//            case 0:
//                ioSystem.write_block(block[OFT[OFT_index].index % 4].index1, OFT[OFT_index].buffer);
//                break;
//            case 1:
//                ioSystem.write_block(block[OFT[OFT_index].index % 4].index2, OFT[OFT_index].buffer);
//                break;
//            case 2:
//                ioSystem.write_block(block[OFT[OFT_index].index % 4].index3, OFT[OFT_index].buffer);
//                break;
//        };
//
//        //Update file length in descriptor
//        block[OFT[OFT_index].index % 4].length = OFT[OFT_index].length;
//
//        //Free OFT entry
//        OFT[OFT_index].index = -1;
//        //Return status
//        return 0;
//    }
//    else
//        return -1;
//}
////RETURN: # bytes read
//int FileSystem::read(int OFT_index,         //IN - OFT index
//                     char* mem_area,        //IN - the memory block which it copy to
//                     int count)             //IN - number of bytes to read
//{
//    int inplace_index = OFT[OFT_index].current_pos % 64;
//    int data_block_number;  //The block's number of a file that is currently in the buffer (1, 2 or 3)
//    int current_block;      //The index of the ldisk block that that is currently in buffer (7-63)
//    int next_block;         //The index of the ldisk block that would be next in the buffer (7-63)
//    int bytesRead;
////cout << "***************READ*************\n";
//
//    for(bytesRead = 0;
//        bytesRead < count && (OFT[OFT_index].current_pos + bytesRead) < OFT[OFT_index].length;
//        bytesRead++){
//
//        //If the current read is still inside the buffer
//        if((OFT[OFT_index].current_pos + bytesRead)% 64 != 0 || (OFT[OFT_index].current_pos + bytesRead) == 0) {
//            mem_area[bytesRead] = OFT[OFT_index].buffer[inplace_index + bytesRead];
//        }
//        else{
//            data_block_number = (OFT[OFT_index].current_pos + bytesRead - 1) / 64;
//            fileDescriptor block[4];
//            ioSystem.read_block(OFT[OFT_index].index / 4 + 1, (char*)block);
//            switch(data_block_number){
//                case 0:
//                    current_block = block[OFT[OFT_index].index % 4].index1;
//                    next_block = block[OFT[OFT_index].index % 4].index2;
//                    break;
//                case 1:
//                    current_block = block[OFT[OFT_index].index % 4].index2;
//                    next_block = block[OFT[OFT_index].index % 4].index3;
//                    break;
//                case 2:
//                    current_block = block[OFT[OFT_index].index % 4].index3;
//                    next_block = -1;
//                    break;
//                default:
//                    cerr << "ERROR!!Current block is more than 2\n";
//                    break;
//            };
//
//            ioSystem.write_block(current_block, OFT[OFT_index].buffer);
//            ioSystem.read_block(next_block, OFT[OFT_index].buffer);
//
//            //Since it move on to a new block, inplace_index will have to be -bytesRead
//            // to cancel out +bytesRead and start back at the beginning of buffer
//            inplace_index = -bytesRead;
//            mem_area[bytesRead] = OFT[OFT_index].buffer[inplace_index + bytesRead];
//        }
//    }
//    //current position will be at the one not read yet
//    OFT[OFT_index].current_pos += bytesRead;
//    //cout << "******End read: current pos is: " << OFT[OFT_index].current_pos << "********" << endl;
//    return bytesRead;
//}
//
////RETURN: # bytes written
//int FileSystem::write(  int OFT_index,              //IN - OFT index
//                        char* mem_area,             //IN -
//                        int count)                  //IN -
//{
//    //Compute position in r/w buffer
//    int inplace_index = OFT[OFT_index].current_pos % 64;
//    int data_block_number;  //The block's number of a file that is currently in the buffer (1, 2 or 3)
//    int current_block;      //The index of the ldisk block that that is currently in buffer (7-63)
//    int next_block;         //The index of the ldisk block that would be next in the buffer (7-63)
//    int bytesWrite;
//    //cout << "Start WRITE****************\n";
//
//    for(bytesWrite = 0;
//        bytesWrite < count && (OFT[OFT_index].current_pos + bytesWrite) < MAX_FILE_LENGTH;
//        bytesWrite++){
//
//        //If the current read is still inside the buffer
//        if((OFT[OFT_index].current_pos + bytesWrite)% 64 != 0 || (OFT[OFT_index].current_pos + bytesWrite) == 0) {
//            OFT[OFT_index].buffer[inplace_index + bytesWrite] = mem_area[bytesWrite];
//        }
//        //It will only in else if index is 64n
//        else{
//            cout << "In else\n";
//            next_block = -1;
//            data_block_number = (OFT[OFT_index].current_pos + bytesWrite - 1) / 64;
//            //int fd_block_number = OFT[OFT_index].index / 4;
//            fileDescriptor block[4];
//            ioSystem.read_block((OFT[OFT_index].index / 4) + 1, (char*)block);
//            //cin.ignore(100, '\n');
//            switch(data_block_number){
//                case 0:
//                    current_block = block[OFT[OFT_index].index % 4].index1;
//
//                    if(block[OFT[OFT_index].index % 4].index2 == -1){
//                        //allocate a new block
//                        block[OFT[OFT_index].index % 4].index2 = allocate_ldisk_block();
//                    }
//                    next_block = block[OFT[OFT_index].index % 4].index2;
//                    break;
//                case 1:
//                    current_block = block[OFT[OFT_index].index % 4].index2;
//                    if(block[OFT[OFT_index].index % 4].index3 == -1){
//                        //allocate a new block
//                        block[OFT[OFT_index].index % 4].index3 = allocate_ldisk_block();
//                    }
//                    next_block = block[OFT[OFT_index].index % 4].index3;
//                    break;
//                case 2:
//                    current_block = block[OFT[OFT_index].index % 4].index3;
//                    next_block = -1;
//                    break;
//                default:
//                    cout << "ERROR!!Current block is more than 2\n";
//                    break;
//            };
//            //Update the new data if there a new block allocated
//            ioSystem.write_block((OFT[OFT_index].index / 4) + 1, (char*)block);
//            ioSystem.write_block(current_block, OFT[OFT_index].buffer);
//            if(next_block == -1)
//                cout << "WRITE ERROR!!!! Out of ldisk block\n";
//            else
//                ioSystem.read_block(next_block, OFT[OFT_index].buffer);
//
//            //Since it move on to a new block, inplace_index will have to be -bytesWrite
//            // to cancel out +bytesWrite and start back at the beginning of buffer
//            inplace_index = 0;
//            OFT[OFT_index].buffer[inplace_index + bytesWrite] = mem_area[bytesWrite];
//        }
//    }
//    //current position will be at the one not read yet
//    OFT[OFT_index].current_pos += bytesWrite;
//    //If current_pos < length -> overwrite data -> no need to update length
//    if(OFT[OFT_index].current_pos >= OFT[OFT_index].length)
//        OFT[OFT_index].length += bytesWrite;
//
////cout << "*********END WRITE. Current pos is: " << OFT[OFT_index].current_pos << "**************" << endl;
//    return bytesWrite;
//}
//
//int FileSystem::lseek(int OFT_index,
//                       int pos){
//    int current_pos_block;
//    int new_pos_block = pos / 64;
//    int data_block_number;              //The block's number of a file that is currently in the buffer (1, 2 or 3)
//    int current_block;                  //The index of the ldisk block that that is currently in buffer (7-63)
//    int new_block;                      //The index of the ldisk block that contains the new position (7-63)
////cout << "***In lSeek***\n";
//
//    //Since the current_pos is the one will be read, its not checked if its out of bound yet
//    //-> have to check if its >= 64, if it is then we have to change the block
//    if(OFT[OFT_index].current_pos % 64 != 0 || OFT[OFT_index].current_pos == 0)
//        current_pos_block = OFT[OFT_index].current_pos / 64;
//    else
//        current_pos_block = (OFT[OFT_index].current_pos - 1) / 64;
//
//    if(pos < OFT[OFT_index].length){
//        //If the new position is not within the current block
//        if(current_pos_block != new_pos_block){
//            //Write buffer to block
//            fileDescriptor block[4];
//            ioSystem.read_block(OFT[OFT_index].index / 4 + 1, (char*)block);
//            switch(current_pos_block){
//                case 0:
//                    current_block = block[OFT[OFT_index].index % 4].index1;
//                    break;
//                case 1:
//                    current_block = block[OFT[OFT_index].index % 4].index2;
//                    break;
//                case 2:
//                    current_block = block[OFT[OFT_index].index % 4].index3;
//                    break;
//                default:
//                    cerr << "ERROR!!Current block is more than 2\n";
//                    break;
//            };
//            ioSystem.write_block(current_block, OFT[OFT_index].buffer);
//            //Read the new block
//            data_block_number = pos / 64;
//            switch(data_block_number){
//                case 0:
//                    new_block = block[OFT[OFT_index].index % 4].index1;
//                    break;
//                case 1:
//                    new_block = block[OFT[OFT_index].index % 4].index2;
//                    break;
//                case 2:
//                    new_block = block[OFT[OFT_index].index % 4].index3;
//                    break;
//                default:
//                    cerr << "ERROR!!Current block is more than 2\n";
//                    break;
//            };
//            ioSystem.read_block(new_block,OFT[OFT_index].buffer);
//        }
//        OFT[OFT_index].current_pos = pos;
//        return OFT[OFT_index].current_pos;
//    }else if(pos == 0){
//        OFT[OFT_index].current_pos = 0;
//        return 0;
//    }
//    else{
//        cout << "LSEEK ERROR! New position is outside of file!!\n";
//        cout << "Position is: " << pos << endl;
//        cout << "Length is: " << OFT[OFT_index].length <<  endl;
//        return -1;
//    }
//}
////RETURN: list of files
//string* FileSystem::directory()
//{return nullptr;}
////This function will restore ldisk from a file or create new if no file is given
//void FileSystem::init(string file_name)     //IN - file to restore the ldisk from or create new if there is no file
//{}
//
////This function will save ldisk to a file
//void FileSystem::save(string file_name)     //IN - file to save ldisk to
//{}
//
//int main()
//{
//    FileSystem fileSystem;
//    char name1[] = "Hel";
//    char name2[] = "No";
//    char name3[] = "No1";
//    char name4[] = "No2";
//    char name5[] = "No3";
//    char name6[] = "No4";
//    char name7[] = "No5";
//    char name8[] = "No6";
//    char name9[] = "No7";
//    char name10[] = "No7";
//    int table[3];
//
//    fileSystem.create(name1);
//    fileSystem.create(name2);
//    fileSystem.create(name3);
//    fileSystem.create(name4);
//    fileSystem.create(name5);
//    fileSystem.create(name6);
//    fileSystem.create(name7);
//    fileSystem.create(name8);
//    fileSystem.create(name9);
//    fileSystem.create(name10);
//
//    table[0] = fileSystem.open("No5");
//    table[1] = fileSystem.open("Hel");
//    fileSystem.open("No5");
//    table[2] = fileSystem.open("No7");
//    fileSystem.open("No6");
//
//    fileSystem.write(table[0], "x", 60);
//
//    return 0;
//}
