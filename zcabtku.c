#include <stdio.h> 
#include <string.h>   /* for all the new-fangled string functions */
#include <stdlib.h>     /* malloc, free, rand */

char propositionLetters[4] = "pqr";
int Fsize = 50;
int cases = 6;
int topLevelBinary = 1;
char binConn;
char binPart1[50];
char binPart2[50];

/**
 * Checks if every character in `string` can be found in `filter`
 */
int inStr(char *string, char *filter) {
    int i = 0;
    while (string[i] != '\0') {
        int k = 0;
        int match = 0;
        while (filter[k] != '\0') {
            if (string[i] == filter[k]) match = 1;
            k++;
        }
        if (match == 0) return 0;
        i++;
    }
    return 1;
}

/**
 * Returns 1 for a proposition, 2 for a negated proposition and 0 otherwise
 */
int isProposition(char* formula) {
    int length = strlen(formula);
    if(length == 1) {
        return inStr(formula, propositionLetters);
    }
    if(length == 2 && formula[0] == '-'){
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
    struct tableau *parent;
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
    t->parent = NULL;
    t->middle = NULL;
    t->right = NULL;
    t->left = NULL;
    return t;
}

/**
 * Iterates through the node adding the middle (only) child to leaves
 */
void add1Child(tableau *root, tableau *middle) {
    if (root->middle != NULL) {
        add1Child(root->middle, middle);
        return;
    }
    if (root->left != NULL && root->right != NULL) {
        add1Child(root->left, middle);
        add1Child(root->right, middle);
        return;
    }
    root->middle = middle;
    middle->parent = root;
}

/**
 * Iterates through nodes adding left and right children to leaves
 */
void add2Children(tableau *root, tableau *left, tableau *right) {
    if (root->middle != NULL) {
        add2Children(root->middle, left, right);
        return;
    }
    if (root->left != NULL && root->right != NULL) {
        add2Children(root->left, left, right);
        add2Children(root->right, left, right);
        return;
    }
    root->left = left;
    root->right = right;
    left->parent = root;
    right->parent = root;
}

/**
 * Adds 1 tableau as a single branch
 */
void add1to1(tableau* root, char *formula) {
    tableau *singleTableau = fToTableau(formula);
    add1Child(root, singleTableau);
    addToQueue(singleTableau);
}

/**
 * Adds two tableau as a single branch
 */
void add2to1(tableau* root, char *topFormula, char *bottomFormula) {
    tableau *top = fToTableau(topFormula);
    tableau *bottom = fToTableau(bottomFormula);
    top->middle = bottom;
    bottom->parent = top;
    add1Child(root, top);
    addToQueue(top);
    addToQueue(bottom);
}

/**
 * Adds two tableau as 2 branches
 */
void add2to2(tableau* root, char *leftFormula, char *rightFormula) {
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
 * Recursively analyses a tableau, returns false as soon as an open branch is found, true if there are none
 */
int closedRecursion(tableau *currentTableau, char* propositions, char* negatedPropositions) {
    // Check if the current formula is a propositions or a negated proposition,
    // then update strings if necessary.
    // NOTE: Makes a copy of the string if changes are needed to avoid collision
    int propositionType = isProposition(currentTableau->formula);

    // Are we a leaf node?
    if(propositionType
       && currentTableau->middle == NULL
       && currentTableau->left == NULL
       && currentTableau->right == NULL) {
        if(propositionType == 1 && !inStr(currentTableau->formula, negatedPropositions)) {
            return 1;
        }
        if(propositionType == 2 && !inStr(currentTableau->formula, propositions)) {
            return 1;
        }
        return 0;
    }

    if(propositionType == 1 && !inStr(currentTableau->formula, propositions)) {
        propositions = concat(currentTableau->formula, propositions);
    } else if(propositionType == 2 && !inStr(currentTableau->formula + 1, negatedPropositions)) {
        negatedPropositions = concat(currentTableau->formula + 1, negatedPropositions);
    }

    // Initiate recursion on children where necessary
    if(currentTableau->middle != NULL) {
        return closedRecursion(currentTableau->middle, propositions, negatedPropositions);
    }
    if(currentTableau->left != NULL && currentTableau->right != NULL) {
        if(closedRecursion(currentTableau->left, propositions, negatedPropositions) == 1) return 1;
        if(closedRecursion(currentTableau->right, propositions, negatedPropositions) == 1) return 1;
        return 0;
    }

    // We'd never reach this place unless the tree was malformed.
    return 0;
};

/**
 * Analyses the tableau and returns false as soon as an open branch is found, true if there are none
 */
int closed(tableau *root) {
    char *propositions = malloc(4 * sizeof *propositions);
    propositions[0] = '\0';
    char *negatedPropositions = malloc(4 * sizeof *negatedPropositions);
    negatedPropositions[0] = '\0';
    return closedRecursion(root, propositions, negatedPropositions);
}

/*input a string and check if its a propositional formula */
int main() {

    char *name = malloc(Fsize);
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

        tableau t = {name, NULL, NULL, NULL, NULL};

        /*expand the root, recursively complete the children*/
        if (parse(name) != 0) {
            complete(&t);
            if (closed(&t)) fprintf(fpout, "%s is not satisfiable.\n", name);
            else fprintf(fpout, "%s is satisfiable.\n", name);
        } else fprintf(fpout, "I told you, %s is not a formula.\n", name);
    }

    fclose(fp);
    fclose(fpout);
    free(name);

    return (0);
}








