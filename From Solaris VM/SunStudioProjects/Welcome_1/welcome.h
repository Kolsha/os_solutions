/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 1997-2007 Sun Microsystems, Inc. All rights reserved.
 *
 * The contents of this file are subject to the terms of either the GNU
 * General Public License Version 2 only ("GPL") or the Common
 * Development and Distribution License("CDDL") (collectively, the
 * "License"). You may not use this file except in compliance with the
 * License. You can obtain a copy of the License at
 * http://www.netbeans.org/cddl-gplv2.html
 * or nbbuild/licenses/CDDL-GPL-2-CP. See the License for the
 * specific language governing permissions and limitations under the
 * License.  When distributing the software, include this License Header
 * Notice in each file and include the License file at
 * nbbuild/licenses/CDDL-GPL-2-CP.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the GPL Version 2 section of the License file that
 * accompanied this code. If applicable, add the following below the
 * License Header, with the fields enclosed by brackets [] replaced by
 * your own identifying information:
 * "Portions Copyrighted [year] [name of copyright owner]"
 *
 * Contributor(s):
 *
 * The Original Software is NetBeans. The Initial Developer of the Original
 * Software is Sun Microsystems, Inc. Portions Copyright 1997-2007 Sun
 * Microsystems, Inc. All Rights Reserved.
 *
 * If you wish your version of this file to be governed by only the CDDL
 * or only the GPL Version 2, indicate your decision by adding
 * "[Contributor] elects to include this software in this distribution
 * under the [CDDL or GPL Version 2] license." If you do not indicate a
 * single choice of license, a recipient has the option to distribute
 * your version of this file under either the CDDL, the GPL Version 2 or
 * to extend the choice of license to its licensees as provided above.
 * However, if you add GPL Version 2 code and therefore, elected the GPL
 * Version 2 license, then the option applies only if the new code is
 * made subject to such option by the copyright holder.
 */

//
// File:   welcome.h
//

#ifndef _welcome_H
#define	_welcome_H

typedef void *Pointer;

typedef struct tDLNode {
    Pointer data;
    struct tDLNode *next;
    struct tDLNode *prev;
} DLNode;

typedef struct tDLList {
    size_t count;
    struct tDLNode *first;
    struct tDLNode *last;
} DLList;

int dslist_insert(DLList *list, DLNode *sibling, Pointer data);

int dslist_remove(DLList *list, Pointer data);

int dslist_remove_all(DLList *list, Pointer data);

int dslist_reverse(DLList *list);

int dslist_position(DLList *list, DLNode *el);


size_t dslist_length(DLList *list);


Pointer dslist_remove_next(DLList *list, DLNode *sibling);


DLNode *dslist_last(DLList *list);

DLNode *dslist_first(DLList *list);

DLNode *dslist_nth(DLList *list, int n);

DLNode *dslist_find(DLList *haystack, Pointer needle);

DLNode *dslist_find_custom(DLList *haystack, Pointer needle,
                           int (*compare_func)(Pointer a, Pointer b));


DLList *dslist_append(DLList *list, Pointer data);

DLList *dslist_prepend(DLList *list, Pointer data);

DLList *dslist_copy(DLList *list);

DLList *dslist_concat(DLList *list1, DLList *list2);


void dslist_free(DLList *list);

void dslist_foreach(DLList *list,
                    void (*func)(Pointer data, Pointer user_data), Pointer user_data);




char * read_line();



#endif	/* _welcome_H */

