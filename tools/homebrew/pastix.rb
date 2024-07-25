###
#
#  @file pastix.rb
#  @copyright 2013-2024 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @brief Homebrew formula for PaStiX 6.*
#
#  @version 6.4.0
#  @author Pierre Ramet
#  @author Mathieu Faverge
#  @date 2024-07-05
#
###
class Pastix < Formula
  desc "Parallel solver for sparse linear systems based on direct methods"
  homepage "https://gitlab.inria.fr/solverstack/pastix"
  url "https://files.inria.fr/pastix/releases/v6/pastix-6.4.0.tar.gz"
  sha256 "891d426188eed56c1075fb34d2d80132593a1536ffc05cf333567f68a4811e55"
  head "https://gitlab.inria.fr/solverstack/pastix.git"
  license "LGPL"

  depends_on "openblas"
  depends_on "openmpi"
  depends_on "gfortran"
  depends_on "hwloc"               # Could be optional but strongly recommanded
  depends_on "scotch"              # Could be optional but strongly recommanded
  depends_on "metis"  => :optional # Enable METIS ordering
  depends_on "starpu" => :optional # Enable STARPU runtime
  depends_on "gcc"    => :build    # GNU Fortran is now provided as part of GCC
  depends_on "cmake"  => :build

  def install
    ENV.append "LDFLAGS", "-Wl,-ld_classic"
    args = ["-DCMAKE_INSTALL_PREFIX=#{prefix}",
            "-DBUILD_SHARED_LIBS=ON",
            "-DBUILD_DOCUMENTATION=OFF",
            "-DPASTIX_INT64=OFF",
            "-DPASTIX_ORDERING_SCOTCH=ON",
            "-DPASTIX_WITH_FORTRAN=ON",
            "-DPASTIX_WITH_MPI=ON",
            "-DPASTIX_WITH_CUDA=OFF",
            "-DPASTIX_WITH_PARSEC=OFF"]
    args += ["-DPASTIX_ORDERING_METIS=ON"] if build.with? "metis"
    args += ["-DPASTIX_WITH_STARPU=ON"] if build.with? "starpu"
    mkdir "build" do
      system "cmake", "..", *args
      system "make"
      system "make", "install"
    end
    pkgshare.install "example" # Contains all test programs.
  end

  def caveats; <<~EOS
    Set the PYTHONPATH environment variable:
      export PYTHONPATH=#{prefix}/lib/python:$PYTHONPATH
    Try python example with:
      python #{prefix}/examples/python/simple.py
    Or simple example with:
      #{prefix}/examples/simple -9 10:10:10
    EOS
  end

  test do
    system "#{prefix}/examples/simple", "-9", "10:10:10"
    ohai "All test output is in #{HOMEBREW_LOGS}/pastix. Please check."
  end
end
