# string to indicate that you want to compile with a dynamic load time
$DYNADEFINE='DYNAHEADER';

# size in bytes of types used in glide.  
# You should fill in this data by compiling and running the output of 
# the program created by subroutine &getbytes()
%typesize=(
           'FxBool' => 4,
           'FxI16' => 4,
           'FxI32' => 4,
           'FxU16' => 4,
           'FxU16*' => 4,
           'FxU32' => 4,
           'FxU32*' => 4,
           'FxU8' => 4,
           'FxU8*' => 4,
           'GrAlphaBlendFnc_t' => 4,
           'GrAlpha_t' => 4,
           'GrAspectRatio_t' => 4,
           'GrBuffer_t' => 4,
           'GrChipID_t' => 4,
           'GrChromakeyMode_t' => 4,
           'GrCmpFnc_t' => 4,
           'GrColorFormat_t' => 4,
           'GrColor_t' => 4,
           'GrCombineFactor_t' => 4,
           'GrCombineFunction_t' => 4,
           'GrCombineLocal_t' => 4,
           'GrCombineOther_t' => 4,
           'GrCullMode_t' => 4,
           'GrDepthBufferMode_t' => 4,
           'GrDitherMode_t' => 4,
           'GrErrorCallbackFnc_t' => 4,
           'GrFogMode_t' => 4,
           'GrFog_t*' => 4,
           'GrHint_t' => 4,
           'GrHwConfiguration*' => 4,
           'GrLOD_t' => 4,
           'GrLfbBypassMode_t' => 4,
           'GrLfbInfo_t*' => 4,
           'GrLfbSrcFmt_t' => 4,
           'GrLfbWriteMode_t' => 4,
           'GrLock_t' => 4,
           'GrMipMapId_t' => 4,
           'GrMipMapMode_t' => 4,
           'GrNCCTable_t' => 4,
           'GrOriginLocation_t' => 4,
           'GrPassthruMode_t' => 4,
           'GrScreenRefresh_t' => 4,
           'GrScreenResolution_t' => 4,
           'GrSmoothingMode_t' => 4,
           'GrSstPerfStats_t*' => 4,
           'GrState*' => 4,
           'GrTexBaseRange_t' => 4,
           'GrTexInfo*' => 4,
           'GrTexTable_t' => 4,
           'GrTextureClampMode_t' => 4,
           'GrTextureCombineFnc_t' => 4,
           'GrTextureFilterMode_t' => 4,
           'GrTextureFormat_t' => 4,
           'GrVertex*' => 4,
           'GuNccTable*' => 4,
           'char*' => 4,
           'float' => 4,
           'int' => 4,
           'int*' => 4,
           'long' => 4,
           'void*' => 4,
           'void**' => 4,
           'GrAlphaSource_t' => 4,
           'GrColorCombineFnc_t' => 4,
           'Gu3dfInfo*' => 4
           );

# this should always start off empty.  It will be filled with missing data types.
%checkarray;


sub slurp_header 
{
    my $headername=shift;
    my @file;
    open(HEADER,"<$headername") || die "Unable to open $headername for input";

    (@file)=<HEADER>;
    close(HEADER);

    \@file;
}

sub parsefunction
{
    my $line=shift;
    my $functionhash=shift;
    my $param;
    my $type;
    my $c;                      # character 
    my $ci;                     # and index
    my $typename;
    my $varname;
    my $bytecount=0;
    my (@typenameRA);

    # turn newlines and semicolons into spaces and squeeze 
    $line =~ tr/\n;/ /ds;       
        
    # Spit the function prototype into name and parameters starting at '('
    my ($head, $tail)=split(/\(/,$line);

    # Remove repeated spaces and tabs
    $head =~ tr/ //s;
    $tail =~ tr/\t\)//ds;
    $tail =~ tr/ //s;

    my (@headarray)=split(/ /,$head);
    my (@tailarray)=split(/,/,$tail);

    foreach $param (@tailarray){
        $param =~ tr/ //s;
        (@typenameRA)=split(/ /,$param);
        $varname=pop(@typenameRA);
        undef $typename;

        # the 'const' keyword does noot change the storage size
        foreach $type (@typenameRA){
            $typename .= $type unless ($type eq 'const');
        }

        # Mush the typename into one big word to make a tidy key
        $typename =~ tr/ //ds;

        # Scan variable name for pointer indicators
        for($ci=0;$ci<length($varname);$ci++){
            $c=substr($varname, $ci, 1);
            if($c eq '[' || $c eq '*'){
                $typename .= '*';
            }
        }

        
        if($typename){
            if(defined($typesize{$typename})){
                $bytecount += $typesize{$typename};
            }
            else{
                printf "Unknown type $typename\n";
                $checkarray{$typename}++;
            }
        }
    }

    # Record the number of bytes on the stack for this function
    # because this changes the name of the function under MS stdcall
    ($functionhash->{$headarray[$#headarray]})=$bytecount;

    # Create the typedef to act as a function prototype 
    # (to typedef the function pointers)
    $line  = 'typedef ' . join(' ',@headarray[ 1 .. ($#headarray-2)]) ;
    $line .= "( $headarray[$#headarray-1]" . ' *' . "$headarray[$#headarray]_fpt )";
    $line .= "($tail);\n";
}

# Function prototypes may span several lines.  
# Create one big line and wrap it # in #ifdef .. #else .. #endif
sub assemble_export 
{
    my $filearray=shift;
    my $functionlist=shift;;

    my $line= shift @$filearray;

    while($line !~ /;/){
        $line .= shift @$filearray;
    }

    my $newline = "#ifdef $DYNADEFINE\n" .  
        &parsefunction($line,$functionlist) . 
            "#else\n\n";

    $newline .= $line . "\n#endif /* $DYNADEFINE */\n";;
}

sub create_init_protos
{
    my $fhash=shift;

    my $retval="\n\n#if defined $DYNADEFINE && ! defined $DYNADEFINE" . 
        "_CREATE_STORAGE \n";

    my $key;

    foreach $key (sort keys %$fhash){
        $retval .= "extern $key" . "_fpt $key;\n";
    }
    $retval .= "extern FxBool LoadGlide();\n";
    $retval .= "#endif /* $DYNADEFINE */\n\n";
}

sub create_init_funcs
{
    my $fhash=shift;
    my $GlideName=shift;

    my $retval="\n\n#define $DYNADEFINE\n";
    $retval .= '/* Do not declare the externs */' . "\n";
    $retval .= "#define $DYNADEFINE" . "_CREATE_STORAGE\n";

    my $key;

    $retval .= "#include <glide.h>\n\n";
    $retval .= "#include <windows.h>\n";
    $retval .= "static HINSTANCE glideDLLInst = NULL;\n";

    foreach $key (sort keys %$fhash){
        $retval .= "$key" . "_fpt $key=NULL;\n";
    }
    $retval .= <<EOFUNC
FxBool LoadGlide()
{
    glideDLLInst = LoadLibrary("$GlideName");
    if(!glideDLLInst) return FALSE;

EOFUNC
    ;

    foreach $key (sort keys %$fhash){
        $retval .= "\t$key = ($key" . "_fpt)GetProcAddress(glideDLLInst,\"";
        $retval .= '_' . $key . '@' . $fhash->{$key} . "\");\n";
        $retval .= "\tif(! $key) return FALSE;\n";
    }

    $retval .= "\n\treturn TRUE;\n}\n";
}


# Create a small C file to get the sizes of all the structures in glide.
sub getbytes
{
    open(CFILE,">getbytes.c") || die "unable to open output file getbytes.c";

    print CFILE <<EOC;
    #include <stdio.h>
    #include <stdlib.h>
    #include <glide.h>
    /* getbytes.c created by $0 */
    int main()
    {
EOC
    ;
    foreach $key (keys %typesize){
        $checkarray{$key}++;
    }

    # NOTE MYSTERY KLUDGE
    # I had been told that the @number suffix on the function name was
    # the actual number of bytes of parameters on the stack, so I thought
    # I could sum the totals returned by sizeof.  However, it looks like
    # even char and 16 bit types like int take 4 bytes on the stack.
    # So I set 4 as the minimum value.  Somehow this makes everything 4 bytes...
    # It looks like I could just multiply the number of arguments by 4 and get
    # it to work, but this hack catches that case and any other permutation.
    # If it ain't broke ... 
    foreach $key (sort keys %checkarray){
        if($key){
            print CFILE 'printf("\'' . $key . '\' => %d,\n", (sizeof(' . $key . ")<4)?4:";
            print CFILE "sizeof( $key ));\n";
        }
    }
    print CFILE "};\n";
    close(CFILE);
}

sub disclaimer
{
    my $filehandle=shift;

    # Add comment to new header to indicate that it has been modified
    my (@datearray)=localtime(time);
    $datearray[4]=
        ('Jan','Feb',
         'Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec')[$datearray[4]];;
    print $filehandle <<EOHACK
/*******************************************************************
 * THIS FILE HAS BEEN HACKED BY AN EXPERIMENTAL PERL SCRIPT
 * $0
 * on $datearray[3] $datearray[4] $datearray[5]
 *
 * The new functionality allows $glidename to be loaded dynamically.
 * To do this, 
 * 1. #define $DYNADEFINE wherever you include the new header ,
 *    $outfile.  
 * 2. Compile and link in glidein.c
 * 3. call LoadGlide();
 */
EOHACK
    ;
    
    
}

sub run
{
    my $infile=shift;
    my $outfile=shift;
    my $glidename=shift;
    my $heteromultihead=shift;

    my %functionlist=();
    my $file=&slurp_header($infile);
    my $line;
    my @datearray;

    open(NEWHEADER,">$outfile") ||
        die "Unable to open $outfile for output";

    &disclaimer(NEWHEADER);

    # Process each line in the source file
    # Assume that all DLL entry points are marked with FX_ENTRY
    do{
        $line=shift @$file;
        if($line =~ /FX_ENTRY/){
            unshift(@$file,$line);
            $line=&assemble_export($file,\%functionlist);
        }
        print NEWHEADER $line;
    }while($#{@$file});
    
    if($heteromultihead){
        print "Heterogeneous multihead not implemented yet\n";
    }
    else{
        print NEWHEADER &create_init_protos(\%functionlist);
    }
    print NEWHEADER $file->[0];       # last line, usually a #endif
    
    close(NEWHEADER);

    # Create Glide inport functions
    open(GLIDEIN,">glidein.c") || die "Unable to open glidein.c";
    &disclaimer(GLIDEIN);
    print GLIDEIN &create_init_funcs(\%functionlist, $glidename);
    close(GLIDEIN);

    if(defined %checkarray){
        print '=' x 60,"\n";
        print <<EOW
        The header file created is useless.  
        The array "typesize" (part of this perl program) was
        missing some of the datatypes used in glide.
        This script will now create a C program to determine
        the data needed for the array typesize.  
        Please compile the program, run it, and 
        capture the output for use as the typesize array.
EOW
    ;
        print '=' x 60,"\n";
        
        &getbytes();

    }


}

# ------------------------------------------------------------
# main()
# ------------------------------------------------------------

$needInfile=1;
$needOutfile=1;

do{
    print "Full path to input glide header:\n";
    $infile=<STDIN>;
    chomp $infile;
    if(! ((-r $infile) && (-T $infile)) ){
        print "$infile does not exist or is not text.  Care to try again? [yn]\n";
        $YN=<STDIN>;
        die "Unable to open input file\n" if($YN !~ /[yY]+/);
        $needInfile=1;
    }
    else {
        $needInfile=0;
    }
}while($needInfile);

do{
    print "Full path to output glide header ($infile is valid):\n";
    $outfile=<STDIN>;
    chomp $outfile;
    if(-r $outfile){
        print "$outfile exists.  Backup and overwrite? [y, n, or quit ]\n";
        $YN=<STDIN>;
        die "Unable to open output file\n" if($YN =~ /[qQ]+/);
        if($YN =~ /[yY]+/){
            my $back=0;
            my $backname=0;
            do{
                $backname=$outfile . "." . $back;
                $backname = 0 if( -e ($backname));
                $back++;
            }while(! $backname);
            system("copy $outfile $backname");
            $needOutfile=0;
        }
        else{
            $needOutfile=1;     
        }
    }
    else {
        $needOutfile=0;
    }
}while($needOutfile);
    
print "Name of glide library to load (path and .dll optional):\n";
$glidename=<STDIN>;
chomp $glidename;

&run($infile, $outfile, $glidename, 0);

if (0){
print <<TODO
-------------------------------------------------------------------------
TO DO:
allow multihead option:
    define structure of all pointers
    make array of max# cards of that structure

make user responsible for loading the library, just pass HINSTANCE to
    getprocaddresses.  Also modify getprocaddresses to work on
    structure within array (maybe pass in array index and HINSTANCE).

Example code: 
    transparent replacement of glide.h
    multihead
    multi source files (to show allocation of storage vs simple prototyping).
-------------------------------------------------------------------------
TODO
    ;
}
