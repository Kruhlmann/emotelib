#include <curl/curl.h>
#include <dirent.h>
#include <libgen.h>
#include <netinet/in.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (ptr == NULL) {
    printf("Not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

char *generate_md5_hash(const char *str) {
  unsigned char digest[MD5_DIGEST_LENGTH];
  MD5((unsigned char *)str, strlen(str), (unsigned char *)&digest);

  char *md5string = (char *)malloc(33);
  for (int i = 0; i < 16; i++) {
    sprintf(&md5string[i * 2], "%02x", (unsigned int)digest[i]);
  }
  return md5string;
}

int download_image(const char *url, const char *output_path) {
  CURL *curl_handle;
  CURLcode res;
  struct MemoryStruct chunk;

  chunk.memory = malloc(1);
  chunk.size = 0;

  curl_global_init(CURL_GLOBAL_ALL);

  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  res = curl_easy_perform(curl_handle);

  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    return -1;
  }

  FILE *fp = fopen(output_path, "wb");
  fwrite(chunk.memory, 1, chunk.size, fp);
  fclose(fp);

  curl_easy_cleanup(curl_handle);
  free(chunk.memory);

  curl_global_cleanup();

  return 0;
}

void process_image(const char *url, const char *filepath,
                   const char *output_dir) {
  char *md5_hash = generate_md5_hash(url);
  char output_file[256];
  snprintf(output_file, sizeof(output_file), "%s/%s.gif", output_dir, md5_hash);
  free(md5_hash);

  if (access(output_file, F_OK) == 0) {
    printf("Image already converted: %s\n", output_file);
    return;
  }

  char command[512];
  snprintf(command, sizeof(command), "convert %s %s", filepath, output_file);

  int result = system(command);
  if (result != 0) {
    printf("Failed to convert image: %s\n", filepath);
  } else {
    printf("Converted and stored image: %s\n", output_file);
  }
}

void load_config_and_process_images(const char *config_path,
                                    const char *output_dir) {
  FILE *config_file = fopen(config_path, "r");
  if (!config_file) {
    perror("Failed to open config file");
    return;
  }

  char url[BUFFER_SIZE];
  while (fgets(url, sizeof(url), config_file)) {
    url[strcspn(url, "\n")] = '\0';
    char output_file[256];
    snprintf(output_file, sizeof(output_file), "%s/tmp_image", output_dir);

    if (download_image(url, output_file) == 0) {
      process_image(url, output_file, output_dir);
      unlink(output_file);
    }
  }

  fclose(config_file);
}

char *generate_index_page(const char *output_dir) {
  char *html = malloc(10240);
  strcpy(html, "<html><head><title>Image "
               "Gallery</title></head><body><h1>Available Images</h1><ul>");

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(output_dir)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type == DT_REG) {
        strcat(html, "<li><a href='");
        strcat(html, ent->d_name);
        strcat(html, "'>");
        strcat(html, ent->d_name);
        strcat(html, "</a></li>");
      }
    }
    closedir(dir);
  } else {
    perror("Failed to read directory");
  }

  strcat(html, "</ul></body></html>");
  return html;
}

void handle_client(int new_socket, const char *output_dir) {
  char buffer[BUFFER_SIZE] = {0};
  read(new_socket, buffer, BUFFER_SIZE);

  char *method = strtok(buffer, " ");
  char *path = strtok(NULL, " ");

  if (method && strcmp(method, "GET") == 0) {
    if (strcmp(path, "/") == 0) {
      char *index_page = generate_index_page(output_dir);
      char response_header[BUFFER_SIZE];
      snprintf(response_header, sizeof(response_header),
               "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: %zu\r\n"
               "Connection: close\r\n\r\n",
               strlen(index_page));

      send(new_socket, response_header, strlen(response_header), 0);
      send(new_socket, index_page, strlen(index_page), 0);
      free(index_page);
    } else {
      char file_path[256];
      snprintf(file_path, sizeof(file_path), "%s%s", output_dir, path);

      FILE *file = fopen(file_path, "rb");
      if (file) {
        struct stat st;
        stat(file_path, &st);
        int file_size = st.st_size;

        char response_header[BUFFER_SIZE];
        snprintf(response_header, sizeof(response_header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n",
                 "image/gif", file_size);

        send(new_socket, response_header, strlen(response_header), 0);

        char file_buffer[BUFFER_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) >
               0) {
          send(new_socket, file_buffer, bytes_read, 0);
        }

        fclose(file);
      } else {
        const char *not_found_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        send(new_socket, not_found_response, strlen(not_found_response), 0);
      }
    }
  }

  close(new_socket);
}

int main(int argc, char const *argv[]) {
  const char *config_path = "/opt/config.txt";
  const char *output_dir = "emotes";

  mkdir(output_dir, 0755);
  load_config_and_process_images(config_path, output_dir);

  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("Listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", PORT);

  while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("Accept failed");
      close(server_fd);
      exit(EXIT_FAILURE);
    }

    handle_client(new_socket, output_dir);
  }

  return 0;
}
