#ifndef SHT_FUNCTIONS_H_
#define SHT_FUNCTIONS_H_

typedef struct{
	Record record;
	int blockId;	//Το block στο οποίο έγινε η εισαγωγή της εγγραφής στο πρωτεύον ευρετήριο.
}SecondaryRecord;

typedef struct{
	int fileDesc;/* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */
	char* attrName;/* το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
	int attrLength;/* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
	long int numBuckets;/* το πλήθος των “κάδων” του αρχείου κατακερματισμού */
	char *fileName; /* όνομα αρχείου με το πρωτεύον ευρετήριο στο id */
}SHT_info;

int SHT_CreateSecondaryIndex(char *sfileName, char* attrName, int attrLength, int buckets, char* fileName);
SHT_info* SHT_OpenSecondaryIndex(char* sfileName);
int SHT_CloseSecondaryIndex(SHT_info* header_info);
int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record);
int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void* value);

int shash_function(char* str,int buckets);

int HashStatistics(char* filename);

#endif