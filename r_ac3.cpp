/*
  mkvmerge -- utility for splicing together matroska files
      from component media subtypes

  r_ac3.h

  Written by Moritz Bunkus <moritz@bunkus.org>

  Distributed under the GPL
  see the file COPYING for details
  or visit http://www.gnu.org/copyleft/gpl.html
*/

/*!
    \file
    \version \$Id: r_ac3.cpp,v 1.12 2003/04/18 10:08:24 mosu Exp $
    \brief AC3 demultiplexer module
    \author Moritz Bunkus         <moritz @ bunkus.org>
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

extern "C" {
#include <avilib.h>
}

#include "mkvmerge.h"
#include "common.h"
#include "error.h"
#include "queue.h"
#include "r_ac3.h"
#include "p_ac3.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

int ac3_reader_c::probe_file(FILE *file, int64_t size) { 
  char         buf[4096];
  int          pos;
  ac3_header_t ac3header;
  
  if (size < 4096)
    return 0;
  if (fseek(file, 0, SEEK_SET) != 0)
    return 0;
  if (fread(buf, 1, 4096, file) != 4096) {
    fseek(file, 0, SEEK_SET);
    return 0;
  }
  fseek(file, 0, SEEK_SET);
  
  pos = find_ac3_header((unsigned char *)buf, 4096, &ac3header);
  if (pos < 0)
    return 0;
  
  return 1;    
}

ac3_reader_c::ac3_reader_c(track_info_t *nti) throw (error_c):
  generic_reader_c(nti) {
  int          pos;
  ac3_header_t ac3header;
  
  if ((file = fopen(ti->fname, "r")) == NULL)
    throw error_c("ac3_reader: Could not open source file.");
  if (fseek(file, 0, SEEK_END) != 0)
    throw error_c("ac3_reader: Could not seek to end of file.");
  size = ftell(file);
  if (fseek(file, 0, SEEK_SET) != 0)
    throw error_c("ac3_reader: Could not seek to beginning of file.");
  chunk = (unsigned char *)malloc(4096);
  if (chunk == NULL)
    die("malloc");
  if (fread(chunk, 1, 4096, file) != 4096)
    throw error_c("ac3_reader: Could not read 4096 bytes.");
  if (fseek(file, 0, SEEK_SET) != 0)
    throw error_c("ac3_reader: Could not seek to beginning of file.");
  pos = find_ac3_header(chunk, 4096, &ac3header);
  if (pos < 0)
    throw error_c("ac3_reader: No valid AC3 packet found in the first " \
                  "4096 bytes.\n");
  bytes_processed = 0;
  ac3packetizer = new ac3_packetizer_c(ac3header.sample_rate,
                                       ac3header.channels, ti);
  if (verbose)
    fprintf(stdout, "Using AC3 demultiplexer for %s.\n+-> Using " \
            "AC3 output module for audio stream.\n", ti->fname);
}

ac3_reader_c::~ac3_reader_c() {
  if (file != NULL)
    fclose(file);
  if (chunk != NULL)
    free(chunk);
  if (ac3packetizer != NULL)
    delete ac3packetizer;
}

int ac3_reader_c::read() {
  int nread;
  
  do {
    if (ac3packetizer->packet_available())
      return EMOREDATA;

    nread = fread(chunk, 1, 4096, file);
    if (nread <= 0)
      return 0;

    ac3packetizer->process(chunk, nread);
    bytes_processed += nread;

  } while (1);
}

packet_t *ac3_reader_c::get_packet() {
  return ac3packetizer->get_packet();
}

int ac3_reader_c::display_priority() {
  return DISPLAYPRIORITY_HIGH - 1;
}

void ac3_reader_c::display_progress() {
  fprintf(stdout, "progress: %lld/%lld bytes (%d%%)\r",
          bytes_processed, size,
          (int)(bytes_processed * 100L / size));
  fflush(stdout);
}

