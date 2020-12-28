#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht_functions.h"
#include "sht_functions.h"
#include "BF.h"

int shash_function(char* str, int buckets){
	
	unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash%buckets;
}

int SHT_CreateSecondaryIndex(char *sfileName, char* attrName, int attrLength, int buckets, char* fileName){

	int i, j, k = 2, count, fileDesc, next_bucket;
	void* block;
	HT_info h;
	SHT_info sh;
	int *shashtable;
	
	sh.attrName = malloc(sizeof(char)*20);
	sh.fileName = malloc(sizeof(char)*20);
	
	shashtable = malloc(buckets*sizeof(int));
	for(i = 0; i < buckets; i++){
		shashtable[i] = i+2;
	}
	
	if(BF_CreateFile(sfileName) == 0){
		fileDesc = BF_OpenFile(sfileName);						//anagnwristiko arxeiou
		if(BF_AllocateBlock(fileDesc) != 0){
			printf("allocate error\n");
			return -1;
		}else{
			if(BF_ReadBlock(fileDesc,0,&block) == 0){					//dimiourgoume to 1o block to opoio exei to anagnwristiko stoixeio katakermatismou k=2  kai ta stoixeia tis domis SHT_info
				memcpy(block,&k,sizeof(int));               		//pername to  anagnwristiko k
				sh.fileDesc = fileDesc;
				strcpy(sh.attrName,attrName);
				sh.attrLength = attrLength;
				sh.numBuckets = buckets;
				strcpy(sh.fileName,fileName);
				memcpy(block+sizeof(int),&sh,sizeof(SHT_info));   //pername ti domi sto block
				if(BF_WriteBlock(fileDesc,0) != 0){
					printf("write error\n");
					return -1;
				}	
			}else{
				printf("read error\n");
				return -1;
			}
		}	
	}else{
		printf("create error\n");
		return -1;

	}
	if(BF_AllocateBlock(fileDesc) != 0){             //desmeush tou 2ou block to arxeiou HT wste na eisagoume ton pinaka hashtable - euretirio sto block
		printf("allocate error\n");
		return -1;
	}else{
		if(BF_ReadBlock(fileDesc,1,&block) == 0){
			memcpy(block,shashtable,buckets*sizeof(int));  //memcpy * buckets wste na perasoume oles tis 8eseis tou eurethriou
			if(BF_WriteBlock(fileDesc,1) != 0){
				printf("write error\n");
				return -1;
			}	
		}else{
			printf("read error\n");
			return -1;
		}
	}
	
	count = 0;	                    //counter eggrafwn
	next_bucket = -1;
	
	for(i = 0; i < buckets; i++){
		if(BF_AllocateBlock(fileDesc) != 0){
			printf("allocate error\n");
			return -1;
		}else{
			int blocknum = BF_GetBlockCounter(fileDesc) - 1;
			if(BF_ReadBlock(fileDesc,blocknum,&block) == 0){
				
				memcpy(block+sizeof(int),&count,sizeof(int));
				memcpy(block+2*sizeof(int),&next_bucket,sizeof(int));
				memcpy(block+3*sizeof(int),&blocknum,sizeof(int));
				if(BF_WriteBlock(fileDesc,blocknum) != 0){
					printf("write error\n");
					return -1;
				}	
			}else{
				printf("read error\n");
				return -1;
			}
		}
	}
	

	int fileDesc2 = BF_OpenFile(fileName);							//anoigoume to primary index
	
	if(fileDesc2 < 0){
		printf("No primary index found\n");
		return -1;
	}	
	
	int pr_buckets, pos;
	int pr_count,pr_next_block,pr_block_id,id;
	
	BF_ReadBlock(fileDesc2,0,&block);								//diavazoume to plithos twn kadwn tou
	memcpy(&h,block+sizeof(int),sizeof(HT_info));
	pr_buckets = h.numBuckets;
	
	for(i = 0; i < pr_buckets; i++){										//gia kathe kado tou
		
		pos = i+2;
		
		do{
			
			BF_ReadBlock(fileDesc2,pos,&block);						//diavasma olwn twn block tou
					
			Record record2;
			SecondaryRecord srecord2;
			
			memcpy(&pr_count,block+sizeof(int),sizeof(int));
			memcpy(&pr_next_block,block+2*sizeof(int),sizeof(int));
			memcpy(&pr_block_id,block+3*sizeof(int),sizeof(int));
			
			for(j = 0; j < pr_count; j++){							//eisagwgi olwn twn eggrafwn tou kathe block sto secondary index
				
				memcpy(&record2,block+3*sizeof(int)+(j+1)*sizeof(Record),sizeof(Record));				
				id = record2.id;
				
				if(id != -1){										//agnooume tis diegrammenes eggrafes
					srecord2.record = record2;
					srecord2.blockId = pr_block_id;
					
					SHT_SecondaryInsertEntry(sh, srecord2);
				}
			}
			
			pos = pr_next_block;
			
		}while(pr_next_block != -1);
		
	}
	
	if(BF_CloseFile(fileDesc) != 0){
		return -1;
	}
	
	return 0;

}

SHT_info* SHT_OpenSecondaryIndex(char* sfileName){

	void * block;
	int k = 2, fileDesc;
	SHT_info *sh;
	
	sh = malloc(sizeof(SHT_info));
	fileDesc = BF_OpenFile(sfileName);
	
	if(BF_ReadBlock(fileDesc,0,&block)==0){
		
		if(memcmp(block,&k,sizeof(int))!=0){         //elegxoume an to arxeio einai arxeio deutereuontos katakermatismoy
			return NULL;
		}		
	}else{
		printf("read error\n");
		return NULL;
	}	
	
	if(fileDesc < 0){
		return NULL;
	}else{
		memcpy(sh,block+sizeof(int),sizeof(SHT_info));            //epistrefoume ti domi SHT_info mesw deikti
		return sh;
	}

}

int SHT_CloseSecondaryIndex(SHT_info* header_info){

	if(BF_CloseFile(header_info->fileDesc) == 0){
		free(header_info);                  		//eleutherwnoume ti mnimi tou SHT_info
		return 0;
	}else{
		return -1;
	}
	
}

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record){
	
	int bucket, hash_position, count, i;
	void * block;
	Record * r;
	
	bucket = shash_function(record.record.name, header_info.numBuckets);
	
	if(BF_ReadBlock(header_info.fileDesc,1,&block) == 0){        //diavazoume apo to 2o block to "bucket" pou mas epestrepse h shash
		memcpy(&hash_position,block+sizeof(int)*bucket,sizeof(int));
	}else{
		printf("read error\n");
		return -1;
	}
	if(BF_ReadBlock(header_info.fileDesc,hash_position,&block)==0){        //diavazoume to sugekrimeno bucket-block
		memcpy(&count,block+sizeof(int),sizeof(int));           //pairnoume ton counter twn eggrafwn tou 
		
		int next_bucket;
		memcpy(&next_bucket,block+2*sizeof(int),sizeof(int));
		
		if(next_bucket == -1){
			
			if(sizeof(SecondaryRecord)*(count+1)+3*sizeof(int)<=512){         //gia na ypologisoume an uparxei xwros gia tin eggrafh tou parontos record, lamvanontas upopsin ta 3 int(count, next_bucket, blocknum)	
				memcpy(block+(count+1)*sizeof(SecondaryRecord)+3*sizeof(int),&record,sizeof(SecondaryRecord));    //an xwraei tote to topothetoume sto telos twn eggrafwn (3*sizeof(int) giati exoume to blocknum, to next_bucket kai counter stin arxi tou block-bucket
				
				count++;
				memcpy(block+sizeof(int),&count,sizeof(int));
				
				if(BF_WriteBlock(header_info.fileDesc, hash_position) != 0){
					printf("write error\n");
					return -1;
				}
			}else{
				if(BF_AllocateBlock(header_info.fileDesc) != 0){
						printf("error\n");
						return -1;
					}
					
					int new_block_num;
					
					
					new_block_num = BF_GetBlockCounter(header_info.fileDesc) - 1;			//pairnoume ton arithmo tou neou block
					
					if(BF_ReadBlock(header_info.fileDesc,hash_position,&block) == 0){        //diavazoume to sugekrimeno bucket-block pou eixe ginei overflow gia na enimerwsoume to next_bucket
						
						memcpy(block+2*sizeof(int),&new_block_num,sizeof(int));
						
						if(BF_WriteBlock(header_info.fileDesc, hash_position)!=0){
							printf("write error\n");
							return -1;
						}
						
					}else{
						printf("read error\n");
						return -1;
					}
					if(BF_ReadBlock(header_info.fileDesc,new_block_num,&block) == 0){		//diavazoume to neo block pou dimiourgithike
			
						int count = 0, next_bucket = -1;
						
						memcpy(block+sizeof(int),&count,sizeof(int));			//arxikopoioume tis times tou
						memcpy(block+2*sizeof(int),&next_bucket,sizeof(int));
						memcpy(block+3*sizeof(int),&new_block_num,sizeof(int));
						
						memcpy(block+(count+1)*sizeof(SecondaryRecord)+3*sizeof(int),&record,sizeof(SecondaryRecord));
						count++;
						memcpy(block+sizeof(int),&count,sizeof(int));
						
						if(BF_WriteBlock(header_info.fileDesc, new_block_num) != 0){
							printf("write error\n");
							return -1;
						}
					}else{
						printf("read error\n");
						return -1;
					}
				}
		}else{                                  //an kados den xwraei alles eggrafes
			while(next_bucket != -1){
				if(BF_ReadBlock(header_info.fileDesc,next_bucket,&block) == 0){        //diavazoume to next bucket
					int pos = next_bucket;
					
					memcpy(&next_bucket,block+2*sizeof(int),sizeof(int));
					if(next_bucket == -1){
						
						memcpy(&count,block+sizeof(int),sizeof(int));
						
						if(sizeof(SecondaryRecord)*(count+1)+3*sizeof(int)<=512){	
							
							memcpy(block+(count+1)*sizeof(SecondaryRecord)+3*sizeof(int),&record,sizeof(SecondaryRecord));
							count++;
							memcpy(block+sizeof(int),&count,sizeof(int));
							if(BF_WriteBlock(header_info.fileDesc, pos) != 0){
								printf("write error\n");
								return -1;
							}					
						}else{
							
							if(BF_AllocateBlock(header_info.fileDesc) != 0){
								printf("error\n");
								return -1;
							}
							
							int new_block_num;
							
							
							new_block_num = BF_GetBlockCounter(header_info.fileDesc) - 1;			//pairnoume ton arithmo tou neou block
							if(BF_ReadBlock(header_info.fileDesc,pos,&block)==0){        //diavazoume to sugekrimeno bucket-block pou eixe ginei overflow gia na enimerwsoume to next_bucket
								
								memcpy(block+2*sizeof(int),&new_block_num,sizeof(int));
								
								if(BF_WriteBlock(header_info.fileDesc, pos)!=0){
									printf("write error\n");
									return -1;
								}
							}else{
								printf("read error\n");
								return -1;
							}
							if(BF_ReadBlock(header_info.fileDesc,new_block_num,&block)==0){		//diavazoume to neo block pou dimiourgithike
					
								int count = 0, next_bucket = -1;
								
								memcpy(block+sizeof(int),&count,sizeof(int));			//arxikopoioume tis times tou
								memcpy(block+2*sizeof(int),&next_bucket,sizeof(int));
								memcpy(block+3*sizeof(int),&new_block_num,sizeof(int));
								memcpy(block+(count+1)*sizeof(SecondaryRecord)+3*sizeof(int),&record,sizeof(SecondaryRecord));
								count++;
								memcpy(block+sizeof(int),&count,sizeof(int));
								
								if(BF_WriteBlock(header_info.fileDesc, new_block_num) != 0){
									printf("write error\n");
									return -1;
								}
								
							}else{
								printf("read error\n");
								return -1;
							}
							
						}
					}
				}else{
					printf("read error\n");
					return -1;
				}
				
			}
		}	
			
	}else{
		printf("read error\n");
		return -1;
	}
	
	return 0;
	
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void* value){

	int count = 0, j, bucket, hash_position, blocks_read = 0, flag = 0;
	int next_block;
	char search_name[15];
	void * block;
	SecondaryRecord srecord;
	
	memcpy(&search_name,value,15*sizeof(char));
	bucket = shash_function(search_name,header_info_sht.numBuckets);
	
	if(BF_ReadBlock(header_info_sht.fileDesc,1,&block)==0){         //diavazoume apo to 1o block to "bucket" pou mas epestrepse h hash
		blocks_read++;
		memcpy(&hash_position,block+sizeof(int)*bucket,sizeof(int));
	}else{
		printf("read error\n");
		return -1;
	}
		
	next_block = 0;
	while(next_block != -1){									//anazitisi se ola ta block gia to id pros diagrafi
		if(BF_ReadBlock(header_info_sht.fileDesc,hash_position,&block) == 0){
			
			blocks_read++;
			
			int blocknum;
			
			memcpy(&blocknum,block+3*sizeof(int),sizeof(int));
			memcpy(&count,block+sizeof(int),sizeof(int));
				
			j = 0;
						
			while(j < count){					//elegxos olwn twn eggrafwn tou sugkekrimenou block
				
				memcpy(&srecord,block+(j+1)*sizeof(SecondaryRecord)+3*sizeof(int),sizeof(SecondaryRecord));
				
				if( strcmp(search_name,srecord.record.name) == 0 ){
					
					flag = 1;
					
					if (j == 0)
						printf("\nSecondary Index Block no: %d , %dst record is:\n",blocknum,j+1);
					else if(j == 1)
						printf("\nSecondary Index Block no: %d , %dnd record is:\n",blocknum,j+1);
					else if(j == 2)
						printf("\nSecondary Index Block no: %d , %drd record is:\n",blocknum,j+1);
					else
						printf("\nSecondary Index Block no: %d , %dth record is:\n",blocknum,j+1);

					printf("\n\t id: %d \n\t name: %s \n\t surname: %s \n\t address: %s\n",srecord.record.id,srecord.record.name,srecord.record.surname,srecord.record.address);
					printf("\nBlocks read: %d\n\n",blocks_read);
					printf("----------------------------------------\n");
					
				}
					
				j++;
			}
							
		}else{
			printf("read error\n");
			return -1;
		}
		
		memcpy(&next_block,block+2*sizeof(int),sizeof(int));				//anazitisi kai sta epomena block
		hash_position = next_block;
		
	}	
	
	if(flag == 1)								//vrethike eggrafi
		return blocks_read;
	else
		return -1;
	
	
}

int HashStatistics(char* filename){
	
	int num_of_blocks = 0;
	int min = 10000, max = -1;
	float average_records, average_blocks;
	int overflow_blocks, overflow_buckets = 0;
	
	int sum_records = 0, sum_blocks = 0;
	int fileDesc, i, buckets, count = 0, next_block;
	void* block;
	int k1 = 1, k2 = 2;									//anagnwristika arxeiwn
	
	fileDesc = BF_OpenFile(filename);
	
	num_of_blocks = BF_GetBlockCounter(fileDesc);				//arithmos block tou arxeiou
	printf("\nFile: %s has %d blocks\n\n", filename, num_of_blocks);
	
	if(BF_ReadBlock(fileDesc,0,&block) == 0){
				
		if(memcmp(block,&k1,sizeof(int)) == 0){         //elegxoume an to arxeio einai arxeio katakermatismoy
			HT_info h;
			memcpy(&h,block+sizeof(int),sizeof(HT_info));
			buckets = h.numBuckets;
		}else if(memcmp(block,&k2,sizeof(int)) == 0){				//elegxoume an to arxeio einai arxeio deutereuontos katakermatismoy
			SHT_info sh;
			memcpy(&sh,block+sizeof(int),sizeof(SHT_info));
			buckets = sh.numBuckets;
		}else
			return -1;			
				
	}else{
		printf("read error\n");
		return -1;
	}
	
	int blocks_per_bucket[buckets];
	int pos;
	
	for(i = 0; i < buckets; i++){
		
		overflow_blocks = 0;
		pos = i+2;
			
		do{
			BF_ReadBlock(fileDesc,pos,&block);
			
			memcpy(&count,block+sizeof(int),sizeof(int));
			if(min > count)
				min = count;
				
			if(max < count)
				max = count;
				
			sum_records += count;
			
			memcpy(&next_block,block+2*sizeof(int),sizeof(int));
			
			pos = next_block;
			
			if(next_block != -1)
				overflow_blocks++;
			
		}while(next_block != -1);
		
		blocks_per_bucket[i] = overflow_blocks;
	}
	
	average_records = (float)sum_records/(float)buckets;
	printf("Average number of records is: %.2f\n", average_records);
	printf("Minimum number of records is: %d\n", min);
	printf("Maximum number of records is: %d\n\n", max);
	
	average_blocks = (float)num_of_blocks/(float)buckets;
	printf("Average number of blocks per bucket is: %.2f\n\n",average_blocks);
	
	for(i = 0; i < buckets; i++){
		
		if(blocks_per_bucket[i] != 0)
			overflow_buckets++;
			
		if(blocks_per_bucket[i] == 0)
			printf("Bucket %d has no overflow blocks\n",i);
		else
			printf("Bucket %d has %d overflow blocks\n",i,blocks_per_bucket[i]);
		
	}
	
	printf("%d buckets had overflow blocks\n\n",overflow_buckets);
	
	return 0;
	
}
