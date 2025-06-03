#include <stdio.h>
#include <string>
#include <format>

#include <pthread.h>

#include <raylib.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <rlImGui.h>

#if defined(_WIN32)           
    #define NOGDI
    #define NOUSER
#endif

#include <enet.h>

#if defined(_WIN32)
    #undef near
    #undef far
#endif

std::string nickname = std::string("");
std::string ipaddress = std::string("127.0.0.1");
int port = 1298;

std::string error_message = std::string("");

bool show_popup_error = false;

bool run_thread = false;
pthread_t enet_thread_hnd;

pthread_t enet_connection_hnd;

ENetHost* client = { 0 };
ENetPeer* peer = { 0 };
ENetAddress address = { 0 };
ENetEvent event;

void* enet_service_thread(void* args) {
    printf("Started the ENet event receiver thread\n");

    ENetEvent event;
    while (run_thread) {
        while (enet_host_service(client, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
                            event.packet -> dataLength,
                            event.packet -> data,
                            event.peer -> data,
                            event.channelID);
             
                    enet_packet_destroy(event.packet);
                    break;
                   
                case ENET_EVENT_TYPE_DISCONNECT:
                    printf ("You have been disconnected.\n");
                    event.peer->data = NULL;
                    enet_peer_reset(peer);
                    peer = NULL;
                    run_thread = false;
                    break;
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    printf("You got disconnected from timeout.");
                    event.peer->data = NULL;
                    enet_peer_reset(peer);
                    peer = NULL;
                    run_thread = false;
                    break;
            }
        }
    }

    printf("Stopped the ENet event receiver thread\n");
    return NULL;
}

void* enet_connection_thread(void* args) {
    printf("Connecting to %s:%d...\n", ipaddress.data(), port);

    enet_address_set_host(&address, ipaddress.data());
    address.port = port;

    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        error_message = std::string("No available peers for initializing an ENet connection.");
        fprintf(stderr, "%s\n", error_message.data());
        show_popup_error = true;
        return NULL;
    }

    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        printf("Connection to %s:%d succeeded.\n", ipaddress.data(), port);
        run_thread = true;
        pthread_create(&enet_thread_hnd, NULL, enet_service_thread, client);

        ENetPacket* packet = enet_packet_create(nickname.data(), nickname.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);

        enet_host_flush(client);
    }
    else {
        error_message = std::string("Connection has failed. Check your internet connection, or if the server is up.");
        fprintf(stderr, "%s\n", error_message.data());
        show_popup_error = true;
        enet_peer_reset(peer);
        peer = NULL;
    }
    return NULL;
}

int main() {
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    client = enet_host_create(NULL, 1, 2, 0, 0);

    if (client == NULL) {
        fprintf(stderr, "An error occurred while trying to create an ENet client host.\n");
        return EXIT_FAILURE;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "client");

    rlImGuiSetup(true);

    printf("Assets path: %s\n", ASSETS_PATH);
    
    ImGuiIO& io = ImGui::GetIO();
    ImFont* robotoFont = io.Fonts->AddFontFromFileTTF(ASSETS_PATH "fonts/Roboto-VariableFont.ttf", 18);
    IM_ASSERT(robotoFont != NULL);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        rlImGuiBegin();
        ImGui::PushFont(robotoFont);

        bool open = true;
        ImGui::ShowDemoWindow(&open);

        if (peer != NULL && peer->state == ENET_PEER_STATE_CONNECTED) {

        }
        else
        {
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();

            ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));

            if (ImGui::Begin("Connect to a Server", nullptr, flags)) {
                ImGui::InputText("Nickname", &nickname, 128);
                ImGui::InputText("IP Address", &ipaddress, 128);
                ImGui::InputInt("Port", &port, 0);

                ImGui::SetCursorPosX((ImGui::GetWindowWidth()/2.0f)-(ImGui::CalcTextSize("Connect").x/2.0f));
                if (ImGui::Button("Connect")) {
                    ImGui::OpenPopup("Connecting");
                    pthread_create(&enet_connection_hnd, NULL, enet_connection_thread, NULL);
                }
            }

            ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Connecting", NULL, flags | ImGuiWindowFlags_NoTitleBar)) {
                ImGui::Text("Connecting to %s:%d...", ipaddress.data(), port);
				ImGui::EndPopup();
            }

            if (show_popup_error) {
                ImGui::OpenPopup("Error");
                show_popup_error = false;
            }

            ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Error", NULL, flags)) {
                ImGui::Text(error_message.data());
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2.0f) - (ImGui::CalcTextSize("Ok").x / 2.0f));
                if (ImGui::Button("Ok")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::End();
        }
 
        ImGui::PopFont();
        rlImGuiEnd();

        EndDrawing();
    }

    pthread_join(enet_connection_hnd, NULL);

    if (run_thread) {
        run_thread = false;
        pthread_join(enet_thread_hnd, NULL);
    }
    
    if (peer != NULL) {
        enet_peer_disconnect(peer, 0);

        uint8_t disconnected = false;
        while (enet_host_service(client, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    puts("Disconnection succeeded.");
                    disconnected = true;
                    break;
            }
        }

        if (!disconnected) {
            enet_peer_reset(peer);
        }
    }
    
    enet_host_destroy(client);
    enet_deinitialize();

    rlImGuiShutdown();
    
    CloseWindow();

    return 0;
}
