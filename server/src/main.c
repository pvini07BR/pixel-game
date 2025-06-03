#include <stdio.h>
#include <enet.h>
#include <pthread.h>

//typedef struct {
//
//} PlayerData;

bool run_server = true;

void* service(void* arg) {
    if (arg == NULL) return NULL;

    ENetHost* server = (ENetHost*)arg;
    if (server == NULL) return NULL;

    ENetEvent event;

    while (run_server) {
        while (enet_host_service(server, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("New client connected: %d\n", event.peer->connectID);
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
                            event.packet->dataLength,
                            event.packet->data,
                            event.peer->data,
                            event.channelID);
                    enet_packet_destroy(event.packet);

                    ENetPacket* packet = enet_packet_create("from server", strlen("from server")+1, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);

                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("Peer ID %d disconnected.\n", event.peer->connectID);
                    /* Reset the peer's client information. */
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    printf("Peer ID %d disconnected due to timeout.\n", event.peer->connectID);
                    event.peer->data = NULL;
                    break;
            }
        }
    }
    return NULL;
}

int main() {
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    ENetAddress address = {0};

    address.host = ENET_HOST_ANY;
    address.port = 1298;

    ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);

    if (server == NULL) {
        printf("An error occurred while trying to create an ENet server host.\n");
        return 1;
    }

    printf("Server started. Listening on 127.0.0.1:1298\n");
    printf("Press Enter to stop the server.\n");

    pthread_t thread;
    pthread_create(&thread, NULL, service, server);

    int character = getchar();
    if (character) {
        printf("Stopping server...\n");
        run_server = false;
    }

    for (int i = 0; i < server->connectedPeers; i++) {
        printf("Disconnecting peer: %d\n", server->peers[i].connectID);
        enet_peer_disconnect_now(&server->peers[i], 0);
    }

    pthread_join(thread, NULL);

    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}
