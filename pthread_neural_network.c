#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

// Structure to hold thread data
typedef struct {
    int inputValue;
    int threadId;
} ThreadData;

// Structure to hold UI and thread data
typedef struct {
    GtkWidget *window;
    GtkWidget *num_threads_entry;
    GtkWidget *input_grid;
    GtkWidget *output_text;
    GtkTextBuffer *output_buffer;
    GPtrArray *input_entries;
    pthread_t *threads;
    ThreadData **thread_data;
    int num_threads;
} AppData;

// Global variables with mutex
int globalCounter = 2;
pthread_mutex_t counterMutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void* foo(void* arg);
void update_output(AppData *app, const char *message);
void on_start_button_clicked(GtkButton *button, AppData *app);
void on_num_threads_changed(GtkEntry *entry, AppData *app);
void cleanup(AppData *app);
int validate_inputs(AppData *app);
void display_neural_network_analogy(AppData *app);

// Initialize GTK application
static void activate(GtkApplication *gtk_app, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    GtkWidget *vbox, *button, *label, *scrolled_window, *nn_label;

    // Create main window
    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "Thread Processor with Neural Network Analogy");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 600, 500);

    // Create vertical box layout
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Neural network analogy label
    nn_label = gtk_label_new("This program demonstrates multithreading with a function 'foo'.\n"
                             "Outputs relate to neural network concepts (Perceptron, FF, RNN, etc.).");
    gtk_box_pack_start(GTK_BOX(vbox), nn_label, FALSE, FALSE, 0);

    // Number of threads entry
    label = gtk_label_new("Number of Threads (1-10):");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    app->num_threads_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app->num_threads_entry), "3");
    gtk_box_pack_start(GTK_BOX(vbox), app->num_threads_entry, FALSE, FALSE, 0);
    g_signal_connect(app->num_threads_entry, "changed", G_CALLBACK(on_num_threads_changed), app);

    // Grid for input values
    app->input_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(app->input_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(app->input_grid), 5);
    gtk_box_pack_start(GTK_BOX(vbox), app->input_grid, FALSE, FALSE, 0);

    // Initialize input entries array
    app->input_entries = g_ptr_array_new();

    // Output text area
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    app->output_text = gtk_text_view_new();
    app->output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->output_text));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->output_text), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), app->output_text);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Start button
    button = gtk_button_new_with_label("Start Processing");
    g_signal_connect(button, "clicked", G_CALLBACK(on_start_button_clicked), app);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    // Update input grid for initial number of threads
    on_num_threads_changed(GTK_ENTRY(app->num_threads_entry), app);

    gtk_widget_show_all(app->window);
}

void on_num_threads_changed(GtkEntry *entry, AppData *app) {
    const char *text = gtk_entry_get_text(entry);
    int num_threads = atoi(text);

    if (num_threads < 1 || num_threads > 10) {
        update_output(app, "Error: Number of threads must be between 1 and 10.\n");
        return;
    }

    app->num_threads = num_threads;

    // Clear existing grid
    gtk_container_foreach(GTK_CONTAINER(app->input_grid), (GtkCallback)gtk_widget_destroy, NULL);
    g_ptr_array_set_size(app->input_entries, 0);

    // Create new input fields
    for (int i = 0; i < num_threads; i++) {
        char label_text[32];
        snprintf(label_text, sizeof(label_text), "Thread %d Input:", i);
        GtkWidget *label = gtk_label_new(label_text);
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry), "1");
        gtk_grid_attach(GTK_GRID(app->input_grid), label, 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(app->input_grid), entry, 1, i, 1, 1);
        g_ptr_array_add(app->input_entries, entry);
    }

    gtk_widget_show_all(app->input_grid);
}

void on_start_button_clicked(GtkButton *button, AppData *app) {
    // Validate inputs
    if (validate_inputs(app) != 0) {
        return;
    }

    // Clear output
    gtk_text_buffer_set_text(app->output_buffer, "", -1);

    // Display neural network analogy
    display_neural_network_analogy(app);

    // Allocate threads and thread data
    app->threads = malloc(app->num_threads * sizeof(pthread_t));
    app->thread_data = malloc(app->num_threads * sizeof(ThreadData *));
    if (!app->threads || !app->thread_data) {
        update_output(app, "Error: Memory allocation failed.\n");
        return;
    }

    // Create threads
    for (int i = 0; i < app->num_threads; i++) {
        app->thread_data[i] = malloc(sizeof(ThreadData));
        if (!app->thread_data[i]) {
            update_output(app, "Error: Memory allocation failed.\n");
            cleanup(app);
            return;
        }
        app->thread_data[i]->threadId = i;
        app->thread_data[i]->inputValue = atoi(gtk_entry_get_text(GTK_ENTRY(g_ptr_array_index(app->input_entries, i))));

        if (pthread_create(&app->threads[i], NULL, foo, app->thread_data[i]) != 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Error: Failed to create thread %d.\n", i);
            update_output(app, msg);
            cleanup(app);
            return;
        }
    }

    // Join threads and display results
    for (int i = 0; i < app->num_threads; i++) {
        int *result;
        if (pthread_join(app->threads[i], (void **)&result) != 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Error: Failed to join thread %d.\n", i);
            update_output(app, msg);
            cleanup(app);
            return;
        }
        char msg[128];
        snprintf(msg, sizeof(msg), "Thread %d returned: %d (Neural analogy: Output of a Perceptron)\n", i, *result);
        update_output(app, msg);
        free(result);
    }

    cleanup(app);
}

void* foo(void* arg) {
    ThreadData *data = (ThreadData *)arg;
    int *result = malloc(sizeof(int));
    if (!result) {
        pthread_exit(NULL);
    }

    // Simulate neural network processing (Perceptron-like)
    char msg[128];
    snprintf(msg, sizeof(msg), "Thread %d (Perceptron) received input: %d\n", data->threadId, data->inputValue);
    // Note: UI updates from threads require g_idle_add in a real application

    if (pthread_mutex_lock(&counterMutex) != 0) {
        free(result);
        pthread_exit(NULL);
    }

    // Update global counter (analogous to RNN state retention)
    globalCounter += data->inputValue;
    snprintf(msg, sizeof(msg), "Thread %d (RNN-like) updated global state to: %d\n", data->threadId, globalCounter);

    if (pthread_mutex_unlock(&counterMutex) != 0) {
        free(result);
        pthread_exit(NULL);
    }

    // Return global counter (analogous to Feed Forward output)
    *result = globalCounter;
    pthread_exit(result);
}

void update_output(AppData *app, const char *message) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->output_buffer, &end);
    gtk_text_buffer_insert(app->output_buffer, &end, message, -1);
}

int validate_inputs(AppData *app) {
    const char *num_threads_text = gtk_entry_get_text(GTK_ENTRY(app->num_threads_entry));
    int num_threads = atoi(num_threads_text);
    if (num_threads < 1 || num_threads > 10) {
        update_output(app, "Error: Number of threads must be between 1 and 10.\n");
        return 1;
    }

    for (int i = 0; i < app->num_threads; i++) {
        const char *input_text = gtk_entry_get_text(GTK_ENTRY(g_ptr_array_index(app->input_entries, i)));
        if (strlen(input_text) == 0 || atoi(input_text) <= 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Error: Invalid input for thread %d.\n", i);
            update_output(app, msg);
            return 1;
        }
    }
    return 0;
}

void display_neural_network_analogy(AppData *app) {
    const char *analogy =
        "Neural Network Analogy:\n"
        "- Each thread runs 'foo', similar to a Perceptron processing an input.\n"
        "- Input passing is like Feed Forward (FF) neural network data flow.\n"
        "- Global counter retains state, akin to Recurrent Neural Network (RNN) loops.\n"
        "- Multiple threads interacting resemble GANs with multiple components.\n"
        "- Input processing could be compared to CNNs handling structured data.\n"
        "- Long-term state retention is similar to LSTM functionality.\n"
        "- Encoding/decoding inputs mirrors Auto Encoder (AE) behavior.\n\n";
    update_output(app, analogy);
}

void cleanup(AppData *app) {
    for (int i = 0; i < app->num_threads; i++) {
        if (app->thread_data && app->thread_data[i]) {
            free(app->thread_data[i]);
        }
    }
    if (app->threads) {
        free(app->threads);
    }
    if (app->thread_data) {
        free(app->thread_data);
    }
    app->threads = NULL;
    app->thread_data = NULL;
}

int main(int argc, char *argv[]) {
    GtkApplication *gtk_app;
    AppData app = {0};

    gtk_app = gtk_application_new("com.example.ThreadProcessor", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(activate), &app);
    int status = g_application_run(G_APPLICATION(gtk_app), argc, argv);

    g_ptr_array_free(app.input_entries, TRUE);
    pthread_mutex_destroy(&counterMutex);
    g_object_unref(gtk_app);

    return status;
}