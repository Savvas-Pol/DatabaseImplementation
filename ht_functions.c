#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht_functions.h"
#include "BF.h"

int hash_function(int id, int buckets){
	
	return id%buckets;
	
}

int HT_CreateIndex(char *fileName, char attrType, char* attrName, int attrLength, int buckets){
	
	int i, k = 1, count, fileDesc, next_bucket;
	void* block;
	HT_info h;
	int *hashtable;
	
	h.attrName = malloc(sizeof(char)*20);
	
	hashtable = malloc(buckets*sizeof(int));
	for(i = 0; i < buckets; i++){
		hashtable[i] = i+2;
	}
	
	if(BF_CreateFile(fileName) == 0){
		fileDesc = BF_OpenFile(fileName);						//anagnwristiko arxeiou
		if(BF_AllocateBlock(fileDesc) != 0){
			printf("allocate error\n");
			return -1;
		}else{
			if(BF_ReadBlock(fileDesc,0,&block) == 0){					//dimiourgoume to 1o block to opoio exei to anagnwristiko stoixeio katakermatismou k=1  kai ta stoixeia tis domis HT_info
				memcpy(block,&k,sizeof(int));               		//pername to  anagnwristiko k
				h.fileDesc = fileDesc;
				h.attrType = attrType;
				strcpy(h.attrName,attrName);
				h.attrLength = attrLength;
				h.numBuckets = buckets;
				memcpy(block+sizeof(int),&h,sizeof(HT_info));   //pername ti domi sto block
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
			memcpy(block,hashtable,buckets*sizeof(int));  //memcpy * buckets wste na perasoume oles tis 8eseis tou eurethriou
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
	
	
	if(BF_CloseFile(fileDesc) != 0){
		return -1;
	}
	
	return 0;	
	
}

HT_info* HT_OpenIndex(char* fileName){
	
	void * block;
	int k = 1, fileDesc;
	HT_info *h;
	
	h = malloc(sizeof(HT_info));
	fileDesc = BF_OpenFile(fileName);
	
	if(BF_ReadBlock(fileDesc,0,&block) == 0){
		
		if(memcmp(block,&k,sizeof(int)) != 0){         //elegxoume an to arxeio einai arxeio katakermatismoy
			return NULL;
		}		
	}else{
		printf("read error\n");
		return NULL;
	}	
	
	if(fileDesc < 0){
		return NULL;
	}else{
		memcpy(h,block+sizeof(int),sizeof(HT_info));            //epistrefoume ti domi HT_info mesw deikti
		return h;
	}	
	
}

int HT_CloseIndex(HT_info* header_info){
	
	if(BF_CloseFile(header_info->fileDesc) == 0){
		free(header_info);                  		//eleutherwnoume ti mnimi tou HT_info
		return 0;
	}else{
		return -1;
	}	
}

int HT_InsertEntry(HT_info header_info, Record record){
	
	int bucket, hash_position, count, i;
	void * block;
	Record * r;
	
	bucket = hash_function(record.id, header_info.numBuckets);
	
	if(BF_ReadBlock(header_info.fileDesc,1,&block) == 0){        //diavazoume apo to 2o block to "bucket" pou mas epestrepse h hash
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
			
			if(sizeof(Record)*(count+1)+3*sizeof(int)<=512){         //gia na ypologisoume an uparxei xwros gia tin eggrafh tou parontos record, lamvanontas upopsin ta 3 int(count, next_bucket, blocknum)	
				memcpy(block+(count+1)*sizeof(Record)+3*sizeof(int),&record,sizeof(Record));    //an xwraei tote to topothetoume sto telos twn eggrafwn (3*sizeof(int) giati exoume to blocknum, to next_bucket kai counter stin arxi tou block-bucket
				
				count++;                                                                //kai count*sizeof(Record) gia na ypologisoume to pou tha graftei sti mnimi
				memcpy(block+sizeof(int),&count,sizeof(int));
				
				if(BF_WriteBlock(header_info.fileDesc, hash_position) != 0){
					printf("write error\n");
					return -1;
				}else{
					return hash_position;							//epistrefoume ton arithmo block pou graftike i eggrafi
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
						
						memcpy(block+(count+1)*sizeof(Record)+3*sizeof(int),&record,sizeof(Record));
						count++;
						memcpy(block+sizeof(int),&count,sizeof(int));
						
						if(BF_WriteBlock(header_info.fileDesc, new_block_num) != 0){
							printf("write error\n");
							return -1;
						}else{
							return new_block_num;							//epistrefoume ton arithmo block pou graftike i eggrafi
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
						
						if(sizeof(Record)*(count+1)+3*sizeof(int)<=512){	
							
							memcpy(block+(count+1)*sizeof(Record)+3*sizeof(int),&record,sizeof(Record));
							count++;
							memcpy(block+sizeof(int),&count,sizeof(int));
							if(BF_WriteBlock(header_info.fileDesc, pos) != 0){
								printf("write error\n");
								return -1;
							}else{
								return pos;							//epistrefoume ton arithmo block pou graftike i eggrafi
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
								memcpy(block+(count+1)*sizeof(Record)+3*sizeof(int),&record,sizeof(Record));
								count++;
								memcpy(block+sizeof(int),&count,sizeof(int));
								
								if(BF_WriteBlock(header_info.fileDesc, new_block_num) != 0){
									printf("write error\n");
									return -1;
								}else{
									return new_block_num;							//epistrefoume ton arithmo block pou graftike i eggrafi
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
	
}

int HT_DeleteEntry(HT_info header_info, void* value){
	
	int deleted_id, hash_position, bucket, count, i, found, next_block;
	void * block;
	Record record;
	
	
	memcpy(&deleted_id,value,sizeof(int));
	bucket = hash_function(deleted_id, header_info.numBuckets);
	
	if(BF_ReadBlock(header_info.fileDesc,1,&block)==0){         //diavazoume apo to 1o block to "bucket" pou mas epestrepse h hash
		memcpy(&hash_position,block+sizeof(int)*bucket,sizeof(int));
	}else{
		printf("read error\n");
		return -1;
	}
	
	next_block = 0;
	while(next_block != -1){									//anazitisi se ola ta block gia to id pros diagrafi
		if(BF_ReadBlock(header_info.fileDesc,hash_position,&block)==0){
			memcpy(&count,block+sizeof(int),sizeof(int));
				
			i = 0;
			found = 0;
			while(i < count && found != 1){
				
				memcpy(&record,block+(i+1)*sizeof(Record)+3*sizeof(int),sizeof(Record));
				
				if(record.id == deleted_id){				//markarisma ws diegrammeno
					record.id = -1;
					record.name[0] = '\0';
					record.surname[0] = '\0';
					record.address[0] = '\0';
					memcpy(block+(i+1)*sizeof(Record)+3*sizeof(int),&record,sizeof(Record));
					
					if(BF_WriteBlock(header_info.fileDesc,hash_position)!=0){
						printf("write error\n");
						return -1;
					}
					
					found = 1;			
				}
				
				i++;
			}
			
			if(found == 0){														//an de vrethike sto 1o block psakse sta epomena
				memcpy(&next_block,block+2*sizeof(int),sizeof(int));
				hash_position = next_block;
			}else{
				return 0;														//diagrafike epitixws
			}
				
		}else{
			printf("read error\n");
			return -1;
		}
	}
	
	return -1;						//de vrethike
	
}

int HT_GetAllEntries(HT_info header_info, void* value){
	
	int count = 0, search_id, j, bucket, hash_position, blocks_read = 0;
	int next_block, found;
	void * block;
	Record record;
	
	memcpy(&search_id,value,sizeof(int));
	bucket = hash_function(search_id, header_info.numBuckets);
	
	if(BF_ReadBlock(header_info.fileDesc,1,&block)==0){         //diavazoume apo to 1o block to "bucket" pou mas epestrepse h hash
		blocks_read++;
		memcpy(&hash_position,block+sizeof(int)*bucket,sizeof(int));
	}else{
		printf("read error\n");
		return -1;
	}
		
	next_block = 0;
	while(next_block != -1){									//anazitisi se ola ta block gia to id pros diagrafi
		if(BF_ReadBlock(header_info.fileDesc,hash_position,&block) == 0){
			
			blocks_read++;
			
			int blocknum;
			
			memcpy(&blocknum,block+3*sizeof(int),sizeof(int));
			memcpy(&count,block+sizeof(int),sizeof(int));
				
			j = 0;
			found = 0;
			
			while(j < count && found != 1){					//elegxos olwn twn eggrafwn tou sugkekrimenou block
				
				memcpy(&record,block+(j+1)*sizeof(Record)+3*sizeof(int),sizeof(Record));
				
				if(search_id == record.id){
					
					if (j == 0)
						printf("\nPrimary Index Block no: %d , %dst record is:\n",blocknum,j+1);
					else if(j == 1)
						printf("\nPrimary Index Block no: %d , %dnd record is:\n",blocknum,j+1);
					else if(j == 2)
						printf("\nPrimary Index Block no: %d , %drd record is:\n",blocknum,j+1);
					else
						printf("\nPrimary Index Block no: %d , %dth record is:\n",blocknum,j+1);

					printf("\n\t id: %d \n\t name: %s \n\t surname: %s \n\t address: %s\n",record.id,record.name,record.surname,record.address);
					printf("\nBlocks read: %d\n\n",blocks_read);
					printf("----------------------------------------\n");
					
					found = 1;
				}
					
				j++;
			}
							
		}else{
			printf("read error\n");
			return -1;
		}
		
		if(found == 0){														//an de vrethike sto 1o block psakse sta epomena
			memcpy(&next_block,block+2*sizeof(int),sizeof(int));
			hash_position = next_block;
		}else{
			return blocks_read;														//vrethike
		}
	}	
	
	return -1;
}
