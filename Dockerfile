# Use Ubuntu as the base image
FROM ubuntu:22.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Set environment variables for MySQL
ENV MYSQL_HOST=host.docker.internal
ENV MYSQL_PORT=3306
ENV MYSQL_USER=mzinga
ENV MYSQL_PASSWORD=mzingamaji
ENV MYSQL_DB=mzinga_system

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libpq-dev \
    libmysqlclient-dev \
    wget \
    git \
    python3 \
    pkg-config \
    autoconf \
    libtool \
    && rm -rf /var/lib/apt/lists/*

# Install Boost 1.86
RUN wget https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz \
    && tar xzvf boost_1_86_0.tar.gz \
    && cd boost_1_86_0 \
    && ./bootstrap.sh --prefix=/usr/local \
    && ./b2 install \
    && cd .. \
    && rm -rf boost_1_86_0 boost_1_86_0.tar.gz

# Install ODB
RUN wget https://www.codesynthesis.com/download/odb/2.4/odb_2.4.0-1_amd64.deb \
    && dpkg -i odb_2.4.0-1_amd64.deb \
    && rm odb_2.4.0-1_amd64.deb

# Install ODB libraries
RUN apt-get update && apt-get install -y \
    libodb-dev \
    libodb-pgsql-dev \
    libodb-mysql-dev \
    libodb-boost-dev \
    && rm -rf /var/lib/apt/lists/*

# Clone and install jwt-cpp
RUN git config --global http.postBuffer 524288000
RUN git clone https://github.com/Thalhammer/jwt-cpp.git jwt-cpp-master\
    && cd jwt-cpp-master \
    && mkdir build && cd build \
    && cmake .. \
    && make install

# Set the working directory in the container
WORKDIR /app

# Copy the CMakeLists.txt and source files
COPY CMakeLists.txt .
COPY models/ ./models/
COPY core/ ./core/
COPY controllers/ ./controllers/
COPY middleware/ ./middleware/
COPY http/ ./http/
COPY main.cpp .

# Create a build directory
RUN mkdir build

# Build the project
RUN cd build && cmake .. && make

# Expose the port your application uses (change if needed)
EXPOSE 8080

# Run the application
CMD ["./build/MyProject"]

