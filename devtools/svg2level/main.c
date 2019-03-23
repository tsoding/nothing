#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxml/debugXML.h>

#define RECTS_COUNT 1024
#define TEXTS_COUNT 1024

#ifdef LIBXML_TREE_ENABLED

typedef struct {
    xmlNode **rects;
    size_t rects_count;
    xmlNode **texts;
    size_t texts_count;
} Context;

static void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: ./svg2level <input.svg> <output.txt>");
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

static const xmlChar *color_of_style(const xmlChar *style)
{
    const char *prefix = "fill:#";
    const char *fill = strstr((const char*)style, prefix);
    if (fill == NULL) {
        return NULL;
    }
    return (const xmlChar*)(fill + strlen(prefix));
}

static xmlNode *find_attr_by_name(xmlNode *node, const xmlChar *attr_name)
{
    for (xmlAttr *attr = node->properties; attr; attr = attr->next) {
        if (xmlStrEqual(attr->name, attr_name)) {
            return attr->children;
        }
    }

    return NULL;
}

static xmlNode *find_node_by_id(xmlNode **nodes, size_t n, const xmlChar *id)
{
    for (size_t i = 0; i < n; ++i) {
        xmlNode *idAttr = find_attr_by_name(nodes[i], (const xmlChar*)"id");
        if (idAttr != NULL && xmlStrEqual(idAttr->content, id)) {
            return nodes[i];
        }
    }

    return NULL;
}

static void fail_node(xmlNode *node, const char *message)
{
    fprintf(stderr, "%s\n", message);
    xmlDebugDumpNode(stderr, node, 0);
    exit(-1);
}

static void save_title(Context *context, FILE *output_file)
{
    xmlNode *node = find_node_by_id(context->texts, context->texts_count, (const xmlChar*)"title");
    if (node == NULL) {
        fprintf(stderr, "Could find text node with `title` id\n");
        exit(-1);
    }

    for (xmlNode *iter = node->children; iter; iter = iter->next) {
        fprintf(output_file, "%s", iter->children->content);
    }
    fprintf(output_file, "\n");
}

static void save_background(Context *context, FILE *output_file)
{
    xmlNode *node = find_node_by_id(context->rects, context->rects_count, (const xmlChar*)"background");
    if (node == NULL) {
        fprintf(stderr, "Could find rect node with `background` id\n");
        exit(-1);
    }

    xmlNode *styleAttr = find_attr_by_name(node, (const xmlChar*)"style");
    if (styleAttr == NULL) {
        fail_node(node, "Background doesn't have 'style' attr");
    }

    const xmlChar *color = color_of_style(styleAttr->content);
    if (color == NULL) {
        fail_node(node, "`style` attr does not define the `fill` of the rectangle");
    }

    fprintf(output_file, "%.6s\n", color);
}

static void save_player(Context *context, FILE *output_file)
{
    // TODO(#736): save_player is not implemented
    (void) context;
    (void) output_file;


}

static void save_platforms(Context *context, FILE *output_file)
{
    // TODO(#737): save_platforms is not implemented
    (void) context;
    (void) output_file;
}

static void save_goals(Context *context, FILE *output_file)
{
    // TODO(#738): save_goals is not implemented
    (void) context;
    (void) output_file;
}

static void save_lavas(Context *context, FILE *output_file)
{
    // TODO(#739): save_lavas is not implemented
    (void) context;
    (void) output_file;
}

static void save_backplatforms(Context *context, FILE *output_file)
// TODO(#740): save_backplatforms is not implemented
{
    (void) context;
    (void) output_file;
}

static void save_boxes(Context *context, FILE *output_file)
{
    // TODO(#741): save_boxes is not implemented
    (void) context;
    (void) output_file;
}

static void save_labels(Context *context, FILE *output_file)
{
    // TODO(#742): save_labels is not implemented
    (void) context;
    (void) output_file;
}

static void save_script_regions(Context *context, FILE *output_file)
{
    // TODO(#743): save_script_regions is not implemented
    (void) context;
    (void) output_file;
}

static void save_level(Context *context, FILE *output_file)
{
    save_title(context, output_file);
    save_background(context, output_file);
    save_player(context, output_file);
    save_platforms(context, output_file);
    save_goals(context, output_file);
    save_lavas(context, output_file);
    save_backplatforms(context, output_file);
    save_boxes(context, output_file);
    save_labels(context, output_file);
    save_script_regions(context, output_file);
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

    Context context;
    context.rects = calloc(RECTS_COUNT, sizeof(xmlNode*));
    context.rects_count = xml_nodes(root, (const xmlChar*)"rect", context.rects, RECTS_COUNT);
    context.texts = calloc(TEXTS_COUNT, sizeof(xmlNode*));
    context.texts_count = xml_nodes(root, (const xmlChar*)"text", context.texts, TEXTS_COUNT);

    save_level(&context, output_file);

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
