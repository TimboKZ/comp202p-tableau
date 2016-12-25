#include <stdio.h> 
#include <string.h>   /* for all the new-fangled string functions */
#include <stdlib.h>     /* malloc, free, rand */

char propositionLetters[4] = "pqr";
size_t formulaSize = 50;
int cases = 10;
int topLevelBinary = 1;
char binConn;
char binPart1[50];
char binPart2[50];

/**
 * Checks if the character can be found in `filter`
 */
int charInStr(char c, char *filter) {
    int k = 0;
    int match = 0;
    while (filter[k] != '\0') {
        if (c == filter[k]) match = 1;
        k++;
    }
    return match;
}

/**
 * Checks if every character in `string` can be found in `filter`
 */
int inStr(char *string, char *filter) {
    int i = 0;
    while (string[i] != '\0') {
        if (!charInStr(string[i], filter)) return 0;
        i++;
    }
    return 1;
}

/**
 * Returns 1 for a proposition, 2 for a negated proposition and 0 otherwise
 */
int isProposition(char *formula) {
    int length = strlen(formula);
    if (length == 1) {
        return inStr(formula, propositionLetters);
    }
    if (length == 2 && formula[0] == '-') {
        return inStr(formula + 1, propositionLetters) * 2;
    }
    return 0;
}

/**
 * Returns a substring of a string
 */
char *subStr(char *string, int start, int end) {
    int length = end - start + 2;
    char *result = malloc(length * sizeof *result);
    int k = 0, i;
    for (i = start; i < end + 1; i++) {
        result[k++] = string[i];
    }
    result[k] = '\0';
    return result;
}

/* returns 0 for non-formulas, 1 for atoms, 2 for negations, 3 for binary connective fmlas,
 * 4 for existential and 5 for universal formulas.*/
int parse(char *string) {
    int length = strlen(string);

    //printf("Recurs.: %s\n", string);

    // Check that string is not empty
    if (length == 0) return 0;

    // Check that the amount of brackets is correct
    int totalRoundBrackets = 0;
    int openingBrackets = 0;
    int closingBrackets = 0;
    int i = 0;
    for (i = 0; i < length; i++) {
        char k = string[i];
        if (k == '(' || k == '[') openingBrackets++;
        if (k == ')' || k == ']') closingBrackets++;
        if (k == '(' || k == ')') totalRoundBrackets++;
    }
    if (openingBrackets != closingBrackets) return 0;

    // Check if input is a proposition
    if (length == 1) {
        return isProposition(string);
    }
    // Check if input is a negation
    if (string[0] == '-') {
        if (length < 2) return 0;
        char *strBit = subStr(string, 1, length - 1);
        int result = parse(strBit);
        free(strBit);
        if (result != 0) return 2;
    }
    // Check if input is a binary formula
    if (string[0] == '(' && string[length - 1] == ')') {
        if (length < 5) return 0;
        char *strBit = subStr(string, 1, length - 2);
        int bitLength = strlen(strBit);
        int cursor = 0;
        if (totalRoundBrackets - 2 != 0) {
            int openBrackets = 0;
            int closedBrackets = 0;
            for (cursor = 0; cursor < bitLength; cursor++) {
                char currChar = strBit[cursor];
                if (currChar == '(') openBrackets++;
                if (currChar == ')') closedBrackets++;
                if ((currChar == '^' || currChar == 'v' || currChar == '>') && openBrackets == closedBrackets) break;
            }
        } else {
            for (cursor = 0; cursor < bitLength; cursor++) {
                char currChar = strBit[cursor];
                if (currChar == '^' || currChar == 'v' || currChar == '>') break;
            }
        }
        char resultChar = strBit[cursor];
        char arrayC[] = {resultChar, '\0'};
        if (inStr(arrayC, "^v>") == 0) return 0;
        char *part1 = subStr(strBit, 0, cursor - 1);
        char *part2 = subStr(strBit, cursor + 1, bitLength - 1);
        if (topLevelBinary != 0) {
            binConn = resultChar;
            strcpy(binPart1, part1);
            strcpy(binPart2, part2);
        }
        topLevelBinary = 0;
        int result = parse(part1) * parse(part2);
        free(part1);
        free(part2);
        free(strBit);
        if (result != 0) return 3;
    }
    return 0;

}

/**
 * Concatenates 2 strings into a new string, keeping the order the same
 */
char *concat(char *string1, char *string2) {
    int length1 = strlen(string1);
    int length2 = strlen(string2);
    int length = length1 + length2 + 1;
    char *result = malloc((length) * sizeof *result);
    int k = 0, i;
    for (i = 0; i < length1; i++) {
        result[k++] = string1[i];
    }
    for (i = 0; i < length2; i++) {
        result[k++] = string2[i];
    }
    result[k] = '\0';
    return result;
}

/**
 * Note: If middle child is set (i.e. `t->middle != NULL`) then the other two children are guaranteed to be empty.
 */
struct tableau {
    char *formula;
    struct tableau *left;
    struct tableau *middle;
    struct tableau *right;
};
typedef struct tableau tableau;

/**
 * Used for bread-first search approach
 */
struct node {
    tableau *currentTableau;
    struct node *next;
};
typedef struct node node;

/**
 * Contains the pointer to the last node in the queue
 */
node *lastNode;

/**
 * Adds a new node to the queue and updates the global node reference
 */
void addToQueue(tableau *t) {
    node *n = malloc(sizeof(node));
    n->currentTableau = t;
    n->next = NULL;
    lastNode->next = n;
    lastNode = n;
}

/**
 * Constructs a new tableau from a formula
 */
tableau *fToTableau(char *formula) {
    tableau *t = malloc(sizeof(tableau));
    t->formula = formula;
    t->middle = NULL;
    t->right = NULL;
    t->left = NULL;
    return t;
}

/**
 * Creates a deep copy of the tableau, i.e. creates brand new copies of all objects referenced by
 * the pointers stored inside of the tableau struct (except `parent`)
 */
tableau *deepCopyTableau(tableau *source) {
    char *formula = concat("", source->formula);
    tableau *new = fToTableau(formula);
    if (source->left != NULL) {
        new->left = deepCopyTableau(source->left);
    }
    if (source->middle != NULL) {
        new->middle = deepCopyTableau(source->middle);
    }
    if (source->right != NULL) {
        new->right = deepCopyTableau(source->right);
    }
    return new;
}

/**
 * Recursively frees memory used by different elements of the tableau struct
 */
void freeTableau(tableau *currentTableau) {
    // Do nothing if we were accidentally called on a null pointer
    if (currentTableau == NULL) return;

    if (currentTableau->left != NULL) {
        freeTableau(currentTableau->left);
        currentTableau->left = NULL;
    }

    if (currentTableau->middle != NULL) {
        freeTableau(currentTableau->middle);
        currentTableau->middle = NULL;
    }

    if (currentTableau->right != NULL) {
        freeTableau(currentTableau->right);
        currentTableau->right = NULL;
    }

    if (currentTableau->formula != NULL) {
        currentTableau->formula = NULL;
        free(currentTableau->formula);
    }

    // Kill ourselves
    currentTableau = NULL;
    free(currentTableau);
}

/**
 * Iterates through the node adding the middle (only) child to leaves
 */
void add1Child(tableau *root, tableau *middle) {
    // If current node is node a leaf, just carry on the recursion
    if (root->middle != NULL) {
        add1Child(root->middle, middle);
        return;
    }
    if (root->left != NULL && root->right != NULL) {
        add1Child(root->left, middle);
        add1Child(root->right, middle);
        return;
    }

    // We don't want different branches to share the same pointer, so we create
    // a brand new copy which has nothing else referencing it
    tableau *middleCopy = deepCopyTableau(middle);

    // Add the new copy to the queue. If it had a middle child, add it to the queue also
    addToQueue(middleCopy);
    if (middleCopy->middle != NULL) {
        addToQueue(middleCopy->middle);
    }

    // Add the new copy as a child to the current (leaf) tableau
    root->middle = middleCopy;
}

/**
 * Iterates through nodes adding left and right children to leaves
 */
void add2Children(tableau *root, tableau *left, tableau *right) {
    // If current node is node a leaf, just carry on the recursion
    if (root->middle != NULL) {
        add2Children(root->middle, left, right);
        return;
    }
    if (root->left != NULL && root->right != NULL) {
        add2Children(root->left, left, right);
        add2Children(root->right, left, right);
        return;
    }

    // We don't want different branches to share the same pointers, so we create
    // a brand new copy which has nothing else referencing it
    tableau *leftCopy = deepCopyTableau(left);
    tableau *rightCopy = deepCopyTableau(right);

    // Add new copies to the queue
    addToQueue(leftCopy);
    addToQueue(rightCopy);

    // Add new copies as children to the current (leaf) tableau
    root->left = left;
    root->right = right;
}

/**
 * Adds 1 tableau as a single branch
 */
void add1to1(tableau *root, char *formula) {
    tableau *singleTableau = fToTableau(formula);
    add1Child(root, singleTableau);
    freeTableau(singleTableau);
}

/**
 * Adds two tableau as a single branch
 */
void add2to1(tableau *root, char *topFormula, char *bottomFormula) {
    tableau *top = fToTableau(topFormula);
    tableau *bottom = fToTableau(bottomFormula);
    top->middle = bottom;
    add1Child(root, top);
    freeTableau(top);
}

/**
 * Adds two tableau as 2 branches
 */
void add2to2(tableau *root, char *leftFormula, char *rightFormula) {
    tableau *left = fToTableau(leftFormula);
    tableau *right = fToTableau(rightFormula);
    add2Children(root, left, right);
    addToQueue(left);
    addToQueue(right);
}

/**
 * Applies alpha and beta rules to break down the tableau, and adds any new nodes
 * produced in the process to the queue.
 */
void completeTableau(tableau *currentTableau) {
    printf(currentTableau->formula);
    printf("\n");
    topLevelBinary = 1;
    int type = parse(currentTableau->formula);
    int length = strlen(currentTableau->formula);

    // Proposition, do nothing
    if (type == 1) return;

    // Negation, enhance!
    if (type == 2) {
        int type2 = parse(currentTableau->formula + 1);

        // Negated proposition, do nothing
        if (type2 == 1) return;

        // Double negation, simplify
        if (type2 == 2) {
            char *formula = subStr(currentTableau->formula, 2, length - 2);
            add1to1(currentTableau, formula);
        }

        // Negated binary formula :^)
        if (type2 == 3) {

            // Negated AND
            if (binConn == '^') {
                char *left = concat("-", binPart1);
                char *right = concat("-", binPart2);
                add2to2(currentTableau, left, right);
            }

            // Negated OR
            if (binConn == 'v') {
                char *top = concat("-", binPart1);
                char *bottom = concat("-", binPart2);
                add2to1(currentTableau, top, bottom);
            }

            // Negated implication
            if (binConn == '>') {
                char *top = concat("", binPart1);
                char *bottom = concat("-", binPart2);
                add2to1(currentTableau, top, bottom);
            }

        }
    }

    // Binary formula, enhance!
    if (type == 3) {

        // AND
        if (binConn == '^') {
            char *top = concat("", binPart1);
            char *bottom = concat("", binPart2);
            add2to1(currentTableau, top, bottom);
        }

        // OR
        if (binConn == 'v') {
            char *left = concat("", binPart1);
            char *right = concat("", binPart2);
            add2to2(currentTableau, left, right);
        }

        // Implication
        if (binConn == '>') {
            char *left = concat("-", binPart1);
            char *right = concat("", binPart2);
            add2to2(currentTableau, left, right);
        }

    }
}

/**
 * Dynamically creates a queue complete a tableau tree by iterating over nodes in said queue
 */
void complete(tableau *root) {
    printf(">>> Complete\n");
    // Declare the first node in the queue and set the global reference
    node *currentNode = malloc(sizeof(struct node));
    currentNode->currentTableau = root;
    lastNode = currentNode;

    // Iterate over all elements in the queue
    while (currentNode != NULL) {
        // Complete the tableau in the current node
        completeTableau(currentNode->currentTableau);

        // Move on to the next node in the queue
        currentNode = currentNode->next;
    }
}

/**
 * Recursively analyses a tableau, returns true as soon as an open branch is found, false if there are none
 */
int openRecursion(tableau *currentTableau, char *propositions, char *negatedPropositions) {
    printf(currentTableau->formula);
    printf("\n");
    // Check if the current formula is a propositions or a negated proposition,
    // then update strings if necessary.
    // NOTE: Makes a copy of the string if changes are needed to avoid collision
    int propositionType = isProposition(currentTableau->formula);
    if (propositionType == 1 && !inStr(currentTableau->formula, propositions)) {
        propositions = concat(currentTableau->formula, propositions);
    } else if (propositionType == 2 && !inStr(currentTableau->formula + 1, negatedPropositions)) {
        negatedPropositions = concat(currentTableau->formula + 1, negatedPropositions);
    }

    // Initiate recursion on children where necessary
    if (currentTableau->middle != NULL) {
        return openRecursion(currentTableau->middle, propositions, negatedPropositions);
    }
    if (currentTableau->left != NULL && currentTableau->right != NULL) {
        if (openRecursion(currentTableau->left, propositions, negatedPropositions) == 1) return 1;
        if (openRecursion(currentTableau->right, propositions, negatedPropositions) == 1) return 1;
        return 0;
    }

    // We are a leaf! :o
    // Check if there any contradictions in char arrays
    int i;
    int len = strlen(propositions);
    for (i = 0; i < len; i++) {
        if (charInStr(propositions[i], negatedPropositions)) return 0;
    }

    // Woooo, we have an open branch!
    return 1;
};

/**
 * Analyses the tableau and returns false as soon as an open branch is found, true if there are none
 */
int closed(tableau *root) {
    printf(">>> Closed\n");
    // Create arrays for propositions and negated propositions
    char *propositions = malloc(4 * sizeof *propositions);
    propositions[0] = '\0';
    char *negatedPropositions = malloc(4 * sizeof *negatedPropositions);
    negatedPropositions[0] = '\0';

    // Initiate recursion for open branches
    int closed = !openRecursion(root, propositions, negatedPropositions);

    // Free arrays
    propositions = NULL;
    free(propositions);
    negatedPropositions = NULL;
    free(negatedPropositions);

    // Return our result
    return closed;
}

/*input a string and check if its a propositional formula */
int main() {

    char *name = malloc(formulaSize);
    FILE *fp, *fpout;

    /* reads from input.txt, writes to output.txt*/
    if ((fp = fopen("input.txt", "r")) == NULL) {
        printf("Error opening file");
        exit(1);
    }
    if ((fpout = fopen("output.txt", "w")) == NULL) {
        printf("Error opening file");
        exit(1);
    }

    int j;
    for (j = 0; j < cases; j++) {
        fscanf(fp, "%s", name);/*read formula*/
        switch (parse(name)) {
            case (0):
                fprintf(fpout, "%s is not a formula.  ", name);
                break;
            case (1):
                fprintf(fpout, "%s is a proposition.  ", name);
                break;
            case (2):
                fprintf(fpout, "%s is a negation.  ", name);
                break;
            case (3):
                fprintf(fpout, "%s is a binary.  ", name);
                break;
            default:
                fprintf(fpout, "What the f***!  ");
        }

        /*make new tableau with name at root, no children, no parent*/

        tableau *root = fToTableau(name);

        /*expand the root, recursively complete the children*/
        if (parse(name) != 0) {
            complete(root);
            if (closed(root)) fprintf(fpout, "%s is not satisfiable.\n", name);
            else fprintf(fpout, "%s is satisfiable.\n", name);
            freeTableau(root);
        } else fprintf(fpout, "I told you, %s is not a formula.\n", name);
    }

    fclose(fp);
    fclose(fpout);
    free(name);

    return (0);
}








