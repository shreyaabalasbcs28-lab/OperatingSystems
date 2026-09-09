#include <stdio.h>
#include <limits.h>

struct Process {
    int allocation[10];
    int maximum[10];
    int need[10];
    int finish;
};

// Function declarations
int checkSafety(struct Process p[], int n, int m, int available[], int safeSequence[], int stepContext);
void handleResourceRequest(struct Process p[], int n, int m, int available[]);

int main() {
    int n, m;

    printf("Enter number of processes: ");
    scanf("%d", &n);

    printf("Enter number of resources: ");
    scanf("%d", &m);

    struct Process p[10];
    int totalInstances[10];
    int available[10];
    int safeSequence[10];

    printf("\nEnter Allocation Matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf("%d", &p[i].allocation[j]);
        }
    }

    printf("\nEnter Maximum Matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf("%d", &p[i].maximum[j]);
        }
    }

    printf("\nEnter Total Instances of Resources:\n");
    for (int j = 0; j < m; j++) {
        scanf("%d", &totalInstances[j]);
    }

    // Auto-calculate initial Available Vector = Total Instances - Sum of Allocated
    for (int j = 0; j < m; j++) {
        int sumAllocated = 0;
        for (int i = 0; i < n; i++) {
            sumAllocated += p[i].allocation[j];
        }
        available[j] = totalInstances[j] - sumAllocated;
    }

    // Calculate initial Need Matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            p[i].need[j] = p[i].maximum[j] - p[i].allocation[j];
        }
    }

    // Display basic derived metrics upfront
    printf("\n--- Initialized Data Metrics ---");
    printf("\nCalculated Need Matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d ", p[i].need[j]);
        }
        printf("\n");
    }

    printf("\nDerived Available Vector: [ ");
    for (int j = 0; j < m; j++) {
        printf("%d ", available[j]);
    }
    printf("]\n");

    int choice;
    do {
        printf("\n=================================");
        printf("\n     BANKER'S ALGORITHM MENU     ");
        printf("\n=================================");
        printf("\n1. Check Current System Safety State");
        printf("\n2. Request New Resources for a Process");
        printf("\n3. Exit");
        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                checkSafety(p, n, m, available, safeSequence, 1);
                break;
            case 2:
                handleResourceRequest(p, n, m, available);
                break;
            case 3:
                printf("\nExiting program...\n");
                break;
            default:
                printf("\nInvalid Choice! Please pick an option between 1 and 3.\n");
        }
    } while (choice != 3);

    return 0;
}

// Safety Algorithm Component featuring execution-trace logging
int checkSafety(struct Process p[], int n, int m, int available[], int safeSequence[], int stepContext) {
    int work[10];
    int count = 0;
   
    // Create local copies to avoid corrupting master matrices during system valuation
    int localFinish[10];
    for (int i = 0; i < n; i++) {
        localFinish[i] = 0;
    }

    for (int j = 0; j < m; j++) {
        work[j] = available[j];
    }

    if (stepContext == 1) {
        printf("\n--- Checking Current System Safety State ---\n");
    } else {
        printf("\n--- Checking Safety After Speculative Grant ---\n");
    }

    printf("Initial Work Vector: [ ");
    for (int j = 0; j < m; j++) {
        printf("%d ", work[j]);
    }
    printf("]\n\n");

    while (count < n) {
        int found = 0;

        for (int i = 0; i < n; i++) {
            if (localFinish[i] == 0) {
                int possible = 1;

                for (int j = 0; j < m; j++) {
                    if (p[i].need[j] > work[j]) {
                        possible = 0;
                        break;
                    }
                }

                if (possible) {
                    printf("Step %d: Process P%d finishes execution.\n", count + 1, i);
                    printf("  Current Work Vector  : [ ");
                    for (int j = 0; j < m; j++) {
                        printf("%d ", work[j]);
                    }
                    printf("]\n");
                   
                    for (int j = 0; j < m; j++) {
                        work[j] += p[i].allocation[j];
                    }

                    safeSequence[count] = i;
                    localFinish[i] = 1;

                    printf("  Updated Work Vector  : [ ");
                    for (int j = 0; j < m; j++) {
                        printf("%d ", work[j]);
                    }
                    printf("]\n\n");

                    count++;
                    found = 1;
                    break; // Restart loop search index from i=0
                }
            }
        }

        if (!found) {
            break;
        }
    }

    if (count == n) {
        printf("Result: SYSTEM IS IN A SAFE STATE.\n");
        printf("Safe Sequence: ");
        for (int i = 0; i < n; i++) {
            printf("P%d", safeSequence[i]);
            if (i != n - 1) printf(" -> ");
        }
        printf("\n");
        return 1;
    } else {
        printf("Result: SYSTEM IS IN AN UNSAFE STATE.\n");
        return 0;
    }
}

// Resource Request Algorithm Component with State rollback validation
void handleResourceRequest(struct Process p[], int n, int m, int available[]) {
    int reqPid;
    int request[10];
    int tempSafeSeq[10];

    printf("\nEnter the Process ID making a request (0 to %d): ", n - 1);
    scanf("%d", &reqPid);

    if (reqPid < 0 || reqPid >= n) {
        printf("ERROR: Invalid Process ID!\n");
        return;
    }

    printf("Enter Request Vector for P%d (%d items):\n", reqPid, m);
    for (int j = 0; j < m; j++) {
        scanf("%d", &request[j]);
    }

    // Step 1: Check Request <= Need
    for (int j = 0; j < m; j++) {
        if (request[j] > p[reqPid].need[j]) {
            printf("\nERROR: Process P%d requested more than its declared maximum need constraint!\n", reqPid);
            return;
        }
    }

    // Step 2: Check Request <= Available
    for (int j = 0; j < m; j++) {
        if (request[j] > available[j]) {
            printf("\nWAIT: Process P%d must wait. Insufficient physical instances available right now.\n", reqPid);
            return;
        }
    }

    // Step 3: Speculative Allocation
    for (int j = 0; j < m; j++) {
        available[j] -= request[j];
        p[reqPid].allocation[j] += request[j];
        p[reqPid].need[j] -= request[j];
    }

    // Step 4: Run safety test on the speculatively altered vector configurations
    int isSafe = checkSafety(p, n, m, available, tempSafeSeq, 2);

    if (isSafe) {
        printf("\nSUCCESS: Request granted permanently! P%d securely holds resources.\n", reqPid);
    } else {
        printf("\nREJECTED: Request denied to prevent deadlock risk. Rolling back transaction changes...\n");
        // Rollback speculative changes
        for (int j = 0; j < m; j++) {
            available[j] += request[j];
            p[reqPid].allocation[j] -= request[j];
            p[reqPid].need[j] += request[j];
        }
        printf("Process P%d is placed back into block-wait conditions.\n", reqPid);
    }
}
