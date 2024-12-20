/**  
 * HTTP Client 
 *   
 * Malachi Eberly  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <pthread.h>

#define PORT 8080

// GTK widgets
GtkWidget *window;
GtkWidget *text_view;
GtkTextBuffer *text_buffer;

// Structure to pass messages to the main thread
typedef struct {
    char message[1024];
} ThreadMessage;

// Thread-safe function to append text to the GTK text view
gboolean append_text_to_view(gpointer data) {
    ThreadMessage *msg = (ThreadMessage *)data;
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(text_buffer, &end);
    gtk_text_buffer_insert(text_buffer, &end, msg->message, -1);
    gtk_text_buffer_insert(text_buffer, &end, "\n", -1);
    free(msg); // Free the allocated memory
    return FALSE; // Remove from the idle queue after execution
}

// Server logic in a separate thread
void *server_thread(void *arg) {
    (void)arg; // Mark unused parameter

    int server_fd, new_socket;
    struct sockaddr_in address;
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    // Creating socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return NULL;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return NULL;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return NULL;
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return NULL;
    }

    // Notify the GUI that the server is listening
    ThreadMessage *start_msg = malloc(sizeof(ThreadMessage));
    snprintf(start_msg->message, sizeof(start_msg->message), "Server is listening on port %d...", PORT);
    g_idle_add(append_text_to_view, start_msg);

    socklen_t addrlen = sizeof(address);
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        // Read the client's message
        ssize_t bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the string

            // Ensure the message is truncated
            char truncated_buffer[994]; 
            strncpy(truncated_buffer, buffer, sizeof(truncated_buffer) - 1);
            truncated_buffer[sizeof(truncated_buffer) - 1] = '\0'; // Ensure null termination

            ThreadMessage *msg = malloc(sizeof(ThreadMessage));
            snprintf(msg->message, sizeof(msg->message), "Message received from client: %s", truncated_buffer);
            g_idle_add(append_text_to_view, msg);
        } else {
            ThreadMessage *msg = malloc(sizeof(ThreadMessage));
            snprintf(msg->message, sizeof(msg->message), "Failed to read from client.");
            g_idle_add(append_text_to_view, msg);
        }

        // Send a response
        send(new_socket, hello, strlen(hello), 0);
        ThreadMessage *response_msg = malloc(sizeof(ThreadMessage));
        snprintf(response_msg->message, sizeof(response_msg->message), "Response sent to client.");
        g_idle_add(append_text_to_view, response_msg);

        close(new_socket);
    }

    close(server_fd);
    return NULL;
}

// GTK application activation function
void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data; // Mark unused parameter

    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Server GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    // Create text view for logs
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);

    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Add a scrollable window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);

    // --- Add this section for dark background styling ---
    {
        // Your desired CSS
        const char *css_data =
            "* {\n"
            "   background-color: #2e2e2e;\n"   // Dark background
            "   color: #ffffff;\n"              // Light text
            "}\n";

        GtkCssProvider *css_provider = gtk_css_provider_new();
        GdkScreen *screen = gdk_screen_get_default();
        gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
        gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(css_provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        g_object_unref(css_provider);
    }
    // --- End of styling section ---

    gtk_widget_show_all(window);

    // Start server in a separate thread
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_detach(server_tid);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.server", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

