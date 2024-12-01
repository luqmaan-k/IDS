#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WINDOW_SIZE 10

// Structure to store TCP statistics (example fields)
typedef struct {
  int tcp_in_use;
  int orphaned;
  int tw;
  int alloc;
  int mem;
} TCPStats;

// Circular buffer structure
typedef struct {
  TCPStats entries[WINDOW_SIZE]; // Fixed-size array of TCPStats
  int head;                      // Points to the index of the current position
  int count; // Tracks the number of valid entries in the buffer
} CircularBuffer;

// Initialize the circular buffer
void init_buffer(CircularBuffer *buffer) {
  buffer->head = 0;
  buffer->count = 0;
}

// Add a new entry to the buffer (overwrites old entries when full)
void add_entry(CircularBuffer *buffer, TCPStats new_entry) {
  buffer->entries[buffer->head] = new_entry; // Insert new entry
  buffer->head =
      (buffer->head + 1) % WINDOW_SIZE; // Move head to the next position

  if (buffer->count < WINDOW_SIZE) {
    buffer->count++; // Increment count until the buffer is full
  }
}

// Print the contents of the buffer
void print_buffer(CircularBuffer *buffer) {
  int i, index;
  printf("TCP Stats Window:\n");
  for (i = 0; i < buffer->count; i++) {
    index = (buffer->head + i) % WINDOW_SIZE; // Circular index calculation
    printf("Entry %d - In Use: %d, Orphaned: %d, TW: %d, Alloc: %d, Mem: %d\n",
           i + 1, buffer->entries[index].tcp_in_use,
           buffer->entries[index].orphaned, buffer->entries[index].tw,
           buffer->entries[index].alloc, buffer->entries[index].mem);
  }
}

void read_syn_queue(TCPStats *info) {
  FILE *file;
  char buffer[256];
  (*info).tcp_in_use = 0;
  (*info).orphaned = 0;
  (*info).tw = 0;
  (*info).alloc = 0;
  (*info).mem = 0;
  file = fopen("/proc/net/sockstat", "r");
  if (file == NULL) {
    perror("Failed to open /proc/net/sockstat");
    exit(EXIT_FAILURE);
  }

  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    if (strncmp(buffer, "TCP:", 4) == 0) {
      // Parse the line starting with "TCP:"
      sscanf(buffer, "TCP: inuse %d orphan %d tw %d alloc %d mem %d",
             &(*info).tcp_in_use, &(*info).orphaned, &(*info).tw,
             &(*info).alloc, &(*info).mem);
    }
  }
  fclose(file);
}

TCPStats fetch_tcp_stats() {
  TCPStats stats;

  read_syn_queue(&stats);

  return stats;
}

int get_syn_queue_size() {
  FILE *file;
  int syn_queue_size;

  file = fopen("/proc/sys/net/ipv4/tcp_max_syn_backlog", "r");
  printf("Opened file \n");
  if (file == NULL) {
    perror("Failed to open /proc/sys/net/ipv4/tcp_max_syn_backlog");
    exit(EXIT_FAILURE);
  }

  // Read the SYN queue size from the file
  if (fscanf(file, "%d", &syn_queue_size) != 1) {
    perror("Failed to read SYN queue size");
    fclose(file);
    exit(EXIT_FAILURE);
  }
  printf("Read file \n");

  fclose(file);
  return syn_queue_size;
}

int main() {
  int syn_queue_size = get_syn_queue_size();
  printf("Syn queue global size : %d \n", syn_queue_size);
  CircularBuffer buff;
  init_buffer(&buff);
  TCPStats temp;
  while (1) {
    read_syn_queue(&temp);
    printf("read queue \n with values : %d \n", temp.tcp_in_use);
    add_entry(&buff, temp);
    print_buffer(&buff);
    sleep(1); // Sleep for 5 seconds
  }
  return 0;
}
