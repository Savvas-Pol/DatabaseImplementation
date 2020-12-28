#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "BF.h"
#include "ht_functions.h"
#include "sht_functions.h"

#define buckets 10
#define sBuckets 10

int main(char argc,char** argv){
	
	/*
	How many records to delete.
	*/
	int DeleteRecords=atoi(argv[1]);
	/*
	Init the BF layer.
	*/
	BF_Init();
	/*
	Index parameters.
	*/
	char* fileName="primary.index";
	char attrType='i';
	char* attrName="id";
	int attrLength=4;
	char* sfileName="secondary.index";
	char sAttrType='c';
	char* sAttrName="name";
	int sAttrLength=15;
	
	FILE* fp;
	
	char ch;
	int i, lines = 0;
	
	char* input_file = "./record_examples/records1K.txt";   // pairnei to input arxeio pou theloume na testaroume
	
	if( ( fp = fopen(input_file,"r") ) == NULL)
		printf("Cannot open file\n");
		
	while(!feof(fp)){
		ch = fgetc(fp);
		if(ch == '\n')
			lines++;
	}
	
	fclose(fp);
	
	if( ( fp = fopen(input_file,"r") ) == NULL)
		printf("Cannot open file\n");
		
	printf("Creating primary index\n");
	int createNumCode = HT_CreateIndex(fileName,attrType,attrName,attrLength,buckets);			//create primary index
	if(createNumCode < 0){
		printf("Error creating index.\n");
		return -1;
	}
	
	HT_info* hi;
	
	hi = HT_OpenIndex(fileName);																	//open primary index
	if(hi == NULL){
		printf("Error opening index.\n");
		return -1;
	}
		
	for (i = 0; i < lines; i++){															//insert records from input file
		Record record;
		
		fscanf(fp, "{%d,\"%15[^\"]\",\"%20[^\"]\",\"%40[^\"]\"}\n", &(record).id, record.name, record.surname, record.address);
		HT_InsertEntry(*hi,record);
	}
	
	for (i = 0; i < DeleteRecords; i++){					//diagrafei tis prwtes DeletedRecords eggrafes pou dexetai ws orisma apo ti grammi entolwn
		Record record;
		record.id = i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.address,"address_%d",i);
		
		HT_DeleteEntry(*hi,(void*)&record.id);
		
	}
	
	for (i = 0; i < lines; i++){					//print all records
		Record record;
		record.id = i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.address,"address_%d",i);
		
		HT_GetAllEntries(*hi,(void*)&record.id);
		
	}
	
	printf("Creating secondary index\n");
	int createErrorCode = SHT_CreateSecondaryIndex(sfileName,sAttrName,sAttrLength,sBuckets,fileName);			//create secondary index
	if (createErrorCode < 0){
		printf("Error creating secondary index\n");
		return -1;
	}
	
	SHT_info* shi = SHT_OpenSecondaryIndex(sfileName);							//open secondary index
	
	if(shi == NULL){
		printf("Error opening secondary index\n");
		return -1;
	}
	
	for (i = lines; i < lines*2; i++){									//insert 2*records of input file in both indexes
		Record record;
		record.id = i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.address,"address_%d",i);
	
		int blockId = HT_InsertEntry(*hi,record);
		if (blockId > 0){
			SecondaryRecord sRecord;
			sRecord.record = record;
			sRecord.blockId = blockId;
			SHT_SecondaryInsertEntry(*shi,sRecord);
			
		}
	}
	
	for (i = 0; i < lines*2; i++){							//print all records of secondary index
		Record record;
		record.id = i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.address,"address_%d",i);
		
		SHT_SecondaryGetAllEntries(*shi,*hi,(void*)record.name);
		
	}
	
	HT_CloseIndex(hi);
	SHT_CloseSecondaryIndex(shi);
	
	
	printf("Statistics:HT\n");
	HashStatistics(fileName);
	printf("Statistics:SHT\n");
	HashStatistics(sfileName);
	
	fclose(fp);
	
	return 0;
	
}
