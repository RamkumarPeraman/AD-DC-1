import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class UsersController extends Controller {
  @tracked users = [];
  @tracked selectedUser = null;
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

  constructor() {
    super(...arguments);
    this.fetchUsers();
  }

  @action
  async fetchUsers() {
    try {
      console.log('Fetching users...');
      const response = await fetch(
        `http://localhost:8080/backend_war_exploded/UserServlet?search=${this.searchQuery}&sortBy=${this.sortBy}`,
      );
      if (!response.ok) {
        throw new Error(`Failed to fetch users: ${response.statusText}`);
      }
      const data = await response.json();
      console.log('Fetched users:', data);
      this.users = data;
      this.totalCount = data.length;
    } catch (error) {
      console.error('Error fetching users:', error);
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
  closePopup() {
    this.selectedUser = null;
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

    if (
      !this.firstName ||
      !this.lastName ||
      !this.displayName ||
      !this.mail ||
      !this.description ||
      !this.telephoneNumber
    ) {
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
}
