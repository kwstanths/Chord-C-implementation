#define TCP_BACKLOG 10
#define PORT_BASE 49152

#define INSERT_TYPE 0
#define QUERY_TYPE 1
#define DELETE_TYPE 2
#define QUERY_STAR 3
#define JOIN_TRANSFER 4
#define JOIN_RECEIVE 5
#define DEPART_TRANSFER 6
#define DEPART_RECEIVE 7

/*
* JOIN_TRANSFER receives the successor of the new node along with the new node's id and hostname.
* JOIN_RECEIVE receives the new node along with pairs file-value
* DEPART_TRANSFER receives the leaving node alone
* DEPART_RECEIVE receives the successor of the leaving node along with pairs file-value
*/
