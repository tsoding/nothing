#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>

#define RECTS_COUNT 1024
#define TEXTS_COUNT 1024

#ifdef LIBXML_TREE_ENABLED

static void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: ./svg2level <input.svg> <output.txt>");
}

static void save_title(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_background(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_player(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_platforms(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_goals(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_lavas(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_backplatforms(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_boxes(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_labels(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_script_regions(xmlNode *root, FILE *output_file)
{
    (void) root;
    (void) output_file;
}

static void save_level(xmlNode *root, FILE *output_file)
{
    save_title(root, output_file);
    save_background(root, output_file);
    save_player(root, output_file);
    save_platforms(root, output_file);
    save_goals(root, output_file);
    save_lavas(root, output_file);
    save_backplatforms(root, output_file);
    save_boxes(root, output_file);
    save_labels(root, output_file);
    save_script_regions(root, output_file);
}

static size_t xml_nodes(xmlNode *root, const xmlChar *node_name, xmlNode **rects, size_t n)
{
    (void) node_name;

    if (n == 0) {
        return 0;
    }

    const size_t old_n = n;

    for (xmlNode *iter = root; n > 0 && iter; iter = iter->next) {
        if (iter->type == XML_ELEMENT_NODE && xmlStrEqual(iter->name, node_name)) {
            rects[0] = iter;
            n--;
            rects++;

        }

        const size_t m = xml_nodes(iter->children, node_name, rects, n);

        n -= m;
        rects += m;
    }

    return old_n - n;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        print_usage(stderr);
        return -1;
    }

    LIBXML_TEST_VERSION

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    xmlDoc *doc = xmlReadFile(input_filename, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Could not parse file `%s`\n", input_filename);
        return -1;
    }

    xmlNode *root = xmlDocGetRootElement(doc);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open file `%s`\n", output_filename);
        return -1;
    }

    xmlNode **rects = calloc(RECTS_COUNT, sizeof(xmlNode*));
    const size_t rects_count = xml_nodes(root, (const xmlChar*)"rect", rects, RECTS_COUNT);

    xmlNode **texts = calloc(TEXTS_COUNT, sizeof(xmlNode*));
    const size_t texts_count = xml_nodes(root, (const xmlChar*)"text", texts, TEXTS_COUNT);

    save_level(root, output_file);

    fclose(output_file);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

#else
int main(void) {
    fprintf(stderr, "Tree support not compiled in\n");
    exit(1);
}
#endif
