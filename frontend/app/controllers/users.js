import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class UsersController extends Controller {
  @tracked users = [];
  @tracked selectedUser = null;
  @tracked selectedLastUser = null;
  @tracked sortBy = '';
  @tracked searchQuery = '';
  @tracked totalCount = 0;
  @tracked firstName = '';
  @tracked lastName = '';
  @tracked displayName = '';
  @tracked mail = '';
  @tracked description = '';
  @tracked telephoneNumber = '';
  @tracked isNewUserPopupVisible = false;
  @tracked createUserError = '';
  @tracked isReportPopupVisible = false;
  @tracked userCreationData = {};
  @tracked userDetails = [];
  @tracked isUserDetailsPopupVisible = false;

  constructor() {
    super(...arguments);
    this.fetchUsers();
  }

  @action
  async fetchUsers(params = {}) {
    params.sortBy = this.sortBy;
    params.search = this.searchQuery;
    const query = new URLSearchParams(params).toString();
    const url = `http://localhost:8080/backend_war_exploded/UserServlet?${query}`;

    try {
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(`Failed to fetch users: ${response.statusText}`);
      }
      const data = await response.json();
      this.users = data.users || [];
      this.totalCount = data.totalCount || 0;
    } catch (error) {
      console.error('Error fetching groups:', error);
      this.users = [];
      this.totalCount = 0;
    }
  }

  @action
  async showUserDetails(displayName) {
    try {
      console.log('Fetching user details for:', displayName);
      const response = await fetch(
        `http://localhost:8080/backend_war_exploded/FetchUserData?displayName=${displayName}`,
      );
      if (!response.ok) {
        throw new Error(`Failed to fetch user details: ${response.statusText}`);
      }
      this.selectedUser = await response.json();
      console.log('Fetched user details:', this.selectedUser);
    } catch (error) {
      console.error('Error fetching user details:', error);
      this.selectedUser = null;
    }
  }

  @action
  async showLastModDetails(displayName) {
    console.log('Fetching last modified details for Users:', displayName);
    try {
      const response = await fetch(`http://localhost:8080/backend_war_exploded/FetchLastModUsr?objName=${displayName}`);
      if (!response.ok) {
        throw new Error(`Failed to fetch last modified details: ${response.statusText}`);
      }
      const data = await response.json();
      this.selectedLastUser = {
        name: data.name || 'No Name Found',
        lastModifiedField: data.lastModifiedField || 'No Field Found',
        value: data.value || 'No Value Found',
        uSNChanged: data.uSNChanged || 'No uSNChanged Found',
        whenCreated: data.whenCreated || 'No whenCreated Found',
        whenChanged: data.whenChanged || 'No whenChanged Found',
      };
    } catch (error) {
      console.error('Error fetching last modified details:', error);
    }
    console.log('hi',this.selectedLastUser);
  }

  @action
  closePopup() {
    this.selectedUser = null;
    this.selectedLastUser = null;
  }

  @action
  updateSortBy(event) {
    this.sortBy = event.target.value;
    this.fetchUsers();
  }

  @action
  updateSearchQuery(event) {
    this.searchQuery = event.target.value;
    this.fetchUsers();
  }

  @action
  openNewUserPopup() {
    this.isNewUserPopupVisible = true;
  }

  @action
  closeNewUserPopup() {
    this.isNewUserPopupVisible = false;
    this.firstName = '';
    this.lastName = '';
    this.displayName = '';
    this.mail = '';
    this.description = '';
    this.telephoneNumber = '';
    this.createUserError = '';
  }

  @action
  updateFirstName(event) {
    this.firstName = event.target.value;
  }

  @action
  updateLastName(event) {
    this.lastName = event.target.value;
  }

  @action
  updateDisplayName(event) {
    this.displayName = event.target.value;
  }

  @action
  updateMail(event) {
    this.mail = event.target.value;
  }

  @action
  updateDescription(event) {
    this.description = event.target.value;
  }

  @action
  updateTelephoneNumber(event) {
    this.telephoneNumber = event.target.value;
  }

  @action
  async createUser(event) {
    event.preventDefault();

    if (!this.firstName || !this.lastName || !this.displayName || !this.mail || !this.description || !this.telephoneNumber) {
      alert('All fields are required!');
      return;
    }
    try {
      const response = await fetch(
        'http://localhost:8080/backend_war_exploded/CreateUserServlet',
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            firstName: this.firstName,
            lastName: this.lastName,
            mail: this.mail,
            phnnumber: this.telephoneNumber,
            description: this.description,
            displayname: this.displayName,
          }),
        },
      );

      const result = await response.json();
      if (result.status === 'success') {
        this.fetchUsers();
        this.closeNewUserPopup();
      } else if (result.message.includes('User already exists')) {
        this.createUserError = 'User already exists!';
      } else {
        this.createUserError = 'Failed to create user!';
      }
    } catch (error) {
      console.error('Error:', error);
      this.createUserError = 'Failed to create user!';
    }
  }

  @action
  confirmDelete(displayName) {
    if (confirm(`Are you sure you want to delete the user '${displayName}'?`)) {
      this.deleteUser(displayName);
    }
  }

  @action
  async deleteUser(displayName) {
    try {
      const response = await fetch(
        'http://localhost:8080/backend_war_exploded/DeleteUserServlet',
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({ displayName }),
        },
      );

      const result = await response.json();
      if (result.status === 'success') {
        // Refetch users after successful deletion
        this.fetchUsers();
      } else {
        alert(result.message || 'Failed to delete user!');
      }
    } catch (error) {
      console.error('Error:', error);
      alert('Failed to delete user!');
    }
  }

  @action
  openReportPopup() {
    this.isReportPopupVisible = true;
    this.fetchUserCreationData();
  }

  @action
  closeReportPopup() {
    this.isReportPopupVisible = false;
  }

  @action
  async fetchUserCreationData() {
    const url = `http://localhost:8080/backend_war_exploded/UserCreationReportServlet`;

    try {
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(`Failed to fetch user creation data: ${response.statusText}`);
      }
      const data = await response.json();
      console.log('User creation data:', data);
      this.userCreationData = data.data;
      this.displayUserReportChart();
    } catch (error) {
      console.error('Error fetching user creation data:', error);
    }
  }

  displayUserReportChart() {
    const ctx = document.getElementById('userReportChart').getContext('2d');
    const labels = Object.keys(this.userCreationData);
    const data = Object.values(this.userCreationData).map(item => item.count);

    new Chart(ctx, {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'Users Created',
          data: data,
          borderColor: 'rgba(75, 192, 192, 1)',
          borderWidth: 2,
          fill: false
        }]
      },
      options: {
        scales: {
          x: {
            title: {
              display: true,
              text: 'Days'
            }
          },
          y: {
            title: {
              display: true,
              text: 'Count'
            }
          }
        },
        onClick: (event, elements) => {
          if (elements.length > 0) {
            const index = elements[0]._index;
            const day = labels[index];
            this.showUsersForDay(day);
          }
        },
        tooltips: {
          callbacks: {
            label: (tooltipItem) => {
              const day = labels[tooltipItem.index];
              const count = data[tooltipItem.index];
              return `Date: ${day}\nCount: ${count}`;
            }
          }
        }
      }
    });
  }

  @action
  async showUsersForDay(day) {
    console.log('Fetching users for day:', day);
    try {
      const response = await fetch(
        `http://localhost:8080/backend_war_exploded/FetchUserNamesForDayServlet?day=${day}`,
      );
      if (!response.ok) {
        throw new Error(
          `Failed to fetch users for the day: ${response.statusText}`,
        );
      }
      console.log('Response:', response);
      const userDetails = await response.json();

      console.log('User details:', userDetails);
      this.userDetails = userDetails.Users;
      this.isUserDetailsPopupVisible = true;
    } catch (error) {
      console.error('Error fetching users for the day:', error);
    }

    console.log('Users for day:', day);
    console.log('User details:', this.userDetails);
  }

  @action
  closeUserDetailsPopup() {
    this.isUserDetailsPopupVisible = false;
    this.userDetails = [];
  }
}