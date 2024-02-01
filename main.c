/*
调用百度api翻译命令行软件
1.操作逻辑 done
2.构造url done
3.访问函数
4.解析json数据
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mbedtls/md5.h"
#include <curl/curl.h>
#include "cJSON/cJSON.h"



char *calculate_md5(const char *input) {
	mbedtls_md5_context ctx;
	mbedtls_md5_starts(&ctx);
	mbedtls_md5_update(&ctx, (const unsigned char *)input, strlen(input));
	
	unsigned char md5_binary[16];
	mbedtls_md5_finish(&ctx, md5_binary);

	char *md5_result = (char *)malloc(33);

	if (md5_result == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	//mbedtls_md5_finish(&ctx, (unsigned char *)md5_result);

	for (int i=0; i<16; i++) {
		snprintf(md5_result + 2 * i, 3, "%02x", md5_binary[i]);
	}
	md5_result[32] = '\0';

	//printf("md5=%s\n", md5_result);
	return md5_result;
}
char *cre_url(char userinput[200]) {
	char url[1000] = "https://fanyi-api.baidu.com/api/trans/vip/translate?";
	char *appid = "20240129001953939";
	char *q = userinput;
	char *from = "en";
	char *to = "zh";
	char salt[60];
	int a = rand();
	sprintf(salt, "%d", a);
	char *secret_key = "ZjeHGmsmJqnsR6xlrBTK";

	char sign[120] = "";
	strcat(sign, appid);
	strcat(sign, q);
	strcat(sign, salt);
	strcat(sign, secret_key);
	//printf("%s\n", sign);	
	char *tmp = calculate_md5(sign); 
	//printf("%s\n", tmp);
	strcat(url,"q=");
	strcat(url,q);
	strcat(url,"&from=");
	strcat(url,from);
	strcat(url,"&to=");
	strcat(url,to);
	strcat(url,"&appid=");
	strcat(url,appid);
	strcat(url,"&salt=");
	strcat(url,salt);
	strcat(url,"&sign=");
	strcat(url,tmp);

	//char *my_url = url;
	//printf("url=%s\n", url);
	return strdup(url);


}
struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        // 内存分配失败
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


char *get_json_data(char *url) {
	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;


	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			exit(EXIT_FAILURE);

		}

		curl_easy_cleanup(curl);
	}else {
		fprintf(stderr, "curl_easy_init() failed\n");
		exit(EXIT_FAILURE);
	}

	if (chunk.size == 0) {
		fprintf(stderr, "No data received from the server\n");
		exit(EXIT_FAILURE);
	}

	chunk.memory = realloc(chunk.memory, chunk.size + 1);
	chunk.memory[chunk.size] = '\0';
	

	return chunk.memory;
}

char *extract_dst_from_json(const char *json_data) {
	cJSON *root = cJSON_Parse(json_data);

	if (root == NULL) {
		fprintf(stderr, "json parsing error\n");
		exit(EXIT_FAILURE);
	}
	
	cJSON *trans_result = cJSON_GetObjectItemCaseSensitive(root, "trans_result");

    if (trans_result == NULL || !cJSON_IsArray(trans_result) || cJSON_GetArraySize(trans_result) == 0) {
        fprintf(stderr, "Error extracting 'trans_result' from JSON\n");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON *result_entry = cJSON_GetArrayItem(trans_result, 0);
    cJSON *dst = cJSON_GetObjectItemCaseSensitive(result_entry, "dst");

	if (dst == NULL || !cJSON_IsString(dst)) {
		fprintf(stderr, "Error extracting 'dst' from JSON\n");
		cJSON_Delete(root);
		return NULL;
	}
	
	char *dst_value = strdup(dst->valuestring);

	cJSON_Delete(root);

	return dst_value;
}

int main(void) {
	char userinput[200];
	char exitcode = 'q';
	printf("start fanyi ~~\n");
	while (1) {
		printf("enter your content: ");
		fgets(userinput, sizeof(userinput), stdin);

		if (userinput[0] == exitcode) {
			break;
		}
		
		//char *result = "";
		size_t len = strlen(userinput);
		//printf("len: %d\n", len);

		if (len > 0 && userinput[len-1] == '\n') {
			userinput[len-1] = '\0';
		}

		printf("%s\n", extract_dst_from_json(get_json_data(cre_url(userinput))));
		
		
		//\0的ascll码=0，所有循环条件不成立，不会执行循环
		/*for (int i = 0; userinput[i] != '\0'; i++) {
			printf("%c\n", userinput[i]);
		}
		*/
	}
}
