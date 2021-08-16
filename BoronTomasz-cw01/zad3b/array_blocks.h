#define array_blocks

struct Block{
    char** rows;
    int last;
};

struct ArrayOfBlocks{
    struct Block* blocks;
    int max_size;
    int lastPos;
};

struct ArrayOfBlocks createMainArray(int size);
char** defineSequence(int numOfFiles,char* argv[]);
struct Block mergeFiles (char* file1, char* file2);
struct ArrayOfBlocks mergeAll (int size,int amountOfFiles,char** argv);
int addNewBlockWithNoFile(struct ArrayOfBlocks array,struct Block block);
int addNewBlock(struct ArrayOfBlocks array,char* tmpFileName);
int countLines(struct ArrayOfBlocks array,int blockIndex);
void removeBlock(struct ArrayOfBlocks array,int blockIndex);
void removeRow(struct ArrayOfBlocks array,int blockIndex,int rowIndex);
void printMerged(struct ArrayOfBlocks array);
