#include <stdio.h>
#include <aspell.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
check_arg (int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "%s <html file>\n", argv[0]);
		exit(2);
	}
	else if (access(argv[1], F_OK)) {
		fprintf(stderr, "file %s didn't exist\n", argv[1]);
		exit(2);
	}
}

static void
check_document (AspellSpeller *speller, const char *filename)
{
	AspellCanHaveError    *ret;
	AspellDocumentChecker *checker;
	AspellToken            token;
	FILE                  *doc;
	char                   line[512];
	char                   out[1024] = { '\0' };

	doc = fopen(filename, "r");

	ret = new_aspell_document_checker(speller);

	if (aspell_error(ret) != 0) {
		fprintf(stderr, "Error: %s\n", aspell_error_message(ret));
		fclose(doc);
		exit(1);
	}
	checker = to_aspell_document_checker(ret);

	while (fgets(line, sizeof(line), doc)) {
		int tail = 0;
		aspell_document_checker_process(checker, line, -1);
repeat:
		token = aspell_document_checker_next_misspelling(checker);
		if (token.len == 0) {
			printf("%s%s", out, line + tail);
			memset(out, 0, sizeof(out));
			continue;
		}

		strncat(out, line + tail,         token.offset - tail);
		strncat(out, "\x1b[7m",           5);
		strncat(out, line + token.offset, token.len);
		strncat(out, "\x1b[0m",           5);
		tail = token.len + token.offset;

		goto repeat;
	}

	delete_aspell_document_checker(checker);
	fclose(doc);
}

int
main (int argc, char *argv[])
{
	check_arg(argc, argv);

	AspellSpeller      *speller;
	AspellConfig       *config;
	AspellCanHaveError *ret;

	config = new_aspell_config();
	aspell_config_replace(config, "lang", "en_US");
	aspell_config_replace(config, "mode", "html");

	ret = new_aspell_speller(config);
	if (aspell_error(ret) != 0) {
		fprintf(stderr, "Error: %s\n", aspell_error_message(ret));
		delete_aspell_can_have_error(ret);
		return 2;
	}

	speller = to_aspell_speller(ret);
	check_document(speller, argv[1]);

	delete_aspell_config(config);
	delete_aspell_speller(speller);
	return 0;
}
