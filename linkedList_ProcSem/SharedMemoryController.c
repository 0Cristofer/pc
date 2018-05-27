/* Gerenciador de memoria compartilhada entre processos */
/* Autor: Bruno Cesar, @bcesarg6, bcesar.g6@gmail.com   */
/* 		  Cristofer Alexandre Oswald 				    */
/* Maio de 2018   */

#include "SharedMemoryController.h"

sh_mem_t* createShMem(int n_nodes){
	sh_mem_t* ctrl;

	//Cria a estrutura controladora da memória
	if ((mem_ctrl_id = shmget(IPC_PRIVATE, sizeof(sh_mem_t), (SHM_R | SHM_W))) < 0){
		printf("Falha ao criar estrutura de memória compartilhada\n");
		exit(1);
	}

	if ((ctrl = shmat(mem_ctrl_id, NULL, 0)) == (void*) -1){
		printf("Falha ao ler estrutura de memória compartilhada na criação\n");
		exit(1);
	}
	shmctl(mem_ctrl_id, IPC_RMID, (struct shmid_ds *) NULL);

	ctrl->n_nodes = n_nodes;

	//Cria a memória especificada
	if ((ctrl->node_mem_shmid = shmget(IPC_PRIVATE, (sizeof(LLNode) * n_nodes), (SHM_R | SHM_W))) < 0){
		printf("Falha ao criar memória compartilhada de nós\n");
		exit(1);
	}

	ctrl->mem_ptr = 0;

	//Cria lista de endereços de memória livres
	if ((ctrl->free_list_shmid = shmget(IPC_PRIVATE, (sizeof(free_id_t) * n_nodes), (SHM_R | SHM_W))) < 0){
		printf("Falha ao criar lista de memória livre\n");
		exit(1);
	}

	//Cria estrutura para estatísticas
	if ((ctrl->stats_shmid = shmget(IPC_PRIVATE, sizeof(stats_t), (SHM_R | SHM_W))) < 0){
		printf("Falha ao criar estrutura para estatísticas\n");
		exit(1);
	}

	ctrl->free_list_size = 0;
	ctrl->next_free_id = -1;

	return ctrl;
}

LLNode* shAlloc(int* id){
	static int next_id = 0;

	int mem_id = -1;
	sh_mem_t* ctrl;
	free_id_t* free_list;
  LLNode* node;
	free_id_t next_free_id;

	ctrl = sh_mem_adds.ctrl_add;

	if(ctrl->free_list_size > 0){
		free_list = sh_mem_adds.free_list_add;

		next_free_id = free_list[ctrl->next_free_id];
		ctrl->next_free_id = next_free_id.previous;
		ctrl->free_list_size--;

		mem_id = next_free_id.id;
	}

	node = sh_mem_adds.node_mem_add;

	if(mem_id == -1){
		if(next_id == ctrl->n_nodes){
			printf("Número máximo de nós alcançado\n");
			*id = -1;

			return NULL;
		}

		node = node + ctrl->mem_ptr;
		ctrl->mem_ptr = ctrl->mem_ptr + 1;

		*id = next_id;
		next_id++;
	}
	else{
		node = node + mem_id;
		*id = mem_id;
	}

	return node;
}

LLNode* getNode(int id){
	if(id == -1)
		return NULL;
	return (sh_mem_adds.node_mem_add + id);
}

void shFree(int id){
	void* mem;
	sh_mem_t* ctrl;

	ctrl = sh_mem_adds.ctrl_add;

	mem = sh_mem_adds.free_list_add;

	((free_id_t*)mem)[id].previous = ctrl->next_free_id;
	((free_id_t*)mem)[id].id = id;
	ctrl->next_free_id = id;
	ctrl->free_list_size++;
}

void getMemAdds(int ct){
	sh_mem_t* ctrl;

	if(ct){
		if((sh_mem_adds.ctrl_add = shmat(mem_ctrl_id, NULL, 0)) == (void*) -1){
			printf("Falha ao ler estrutura de memória compartilhada\n");
			exit(1);
		}
		shmctl(mem_ctrl_id, IPC_RMID, (struct shmid_ds *) NULL);
	}
	ctrl = sh_mem_adds.ctrl_add;


	if((sh_mem_adds.free_list_add = shmat(ctrl->free_list_shmid,
																				NULL, 0)) == (void*) -1){
		printf("Falha ao ler lista de memória livre, id: %d\n", ctrl->free_list_shmid);
		exit(1);
	}
	shmctl(ctrl->free_list_shmid, IPC_RMID, (struct shmid_ds *) NULL);

	if((sh_mem_adds.node_mem_add = shmat(ctrl->node_mem_shmid,
																			 NULL, 0)) == (void*) -1){
		printf("Falha ao ler memória compartilhada de nós\n");
		exit(1);
	}
	shmctl(ctrl->node_mem_shmid, IPC_RMID, (struct shmid_ds *) NULL);

	if((sh_mem_adds.stats_add = shmat(ctrl->stats_shmid,
																			 NULL, 0)) == (void*) -1){
		printf("Falha ao ler estrutura de estatísticas\n");
		exit(1);
	}
	shmctl(ctrl->stats_shmid, IPC_RMID, (struct shmid_ds *) NULL);
}
